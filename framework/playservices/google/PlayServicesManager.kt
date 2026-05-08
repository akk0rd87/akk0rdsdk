package org.akkord.lib

import android.app.Activity
import android.util.Log
import com.google.android.gms.games.AuthenticationResult
import com.google.android.gms.games.GamesSignInClient
import com.google.android.gms.games.PlayGames
import com.google.android.gms.games.PlayGamesSdk
import com.google.android.gms.games.SnapshotsClient.DataOrConflict
import com.google.android.gms.games.leaderboard.LeaderboardVariant
import com.google.android.gms.games.snapshot.Snapshot
import com.google.android.gms.games.snapshot.SnapshotMetadata
import com.google.android.gms.games.snapshot.SnapshotMetadataChange
import com.google.android.gms.tasks.Continuation
import com.google.android.gms.tasks.OnCompleteListener
import com.google.android.gms.tasks.Task
import com.google.android.gms.tasks.TaskCompletionSource

class PlayServicesManager(
    private val activity: Activity,
    private val useSnapshot: Boolean,
    private val playServicesObserver: PlayServicesObserver,
) {
    init {
        PlayGamesSdk.initialize(activity)
    }

    private val gamesSignInClient: GamesSignInClient = PlayGames.getGamesSignInClient(activity)
    @Volatile
    private var initialSnapshotLoadStarted = false
    @Volatile
    private var connected = false

    fun loadSnapshot() {
        try {
            Log.d(Utils.TAG, "loadSnapshot")
            waitForClosedAndOpen().addOnCompleteListener { openTask ->
                if (!openTask.isSuccessful) {
                    // STATUS_SNAPSHOT_CONTENTS_UNAVAILABLE (4002) and similar errors land here.
                    // SnapshotCoordinator.createOpenListener already called setClosed(), so state is clean.
                    Utils.handleException(openTask.exception, "loadSnapshot: waitForClosedAndOpen failed")
                    return@addOnCompleteListener
                }
                val result = openTask.result
                if (result != null) {
                    val wasConflict = (if (result.isConflict) 1 else 0)

                    if (wasConflict > 0) {
                        Log.i(Utils.TAG, "IS CONFLICT !!!")
                    }

                    processSnapshotOpenResult(result, 0)
                        .addOnCompleteListener { task ->
                            try {
                                if (!task.isSuccessful) {
                                    Utils.handleException(task.exception, "loadSnapshot: processSnapshotOpenResult failed")
                                    return@addOnCompleteListener
                                }

                                val snapshot = task.getResult()

                                if (snapshot == null) {
                                    Log.w(
                                        Utils.TAG,
                                        "Conflict was not resolved automatically, waiting for user to resolve."
                                    )
                                } else {
                                    try {
                                        //readSavedGame(snapshot);
                                        val data = snapshot.snapshotContents.readFully()
                                        Log.i(Utils.TAG, "Snapshot loaded. Size: " + data.size)
                                        onSnapshotReceived(data)
                                    } catch (e: Exception) {
                                        Log.e(Utils.TAG, "Error while reading snapshot contents: " + e.message)
                                    }

                                    SnapshotCoordinator.getInstance().discardAndClose(getSnapshotsClient(), snapshot)
                                        .addOnFailureListener { e ->
                                            Utils.handleException(e, "There was a problem discarding the snapshot!")
                                        }
                                }
                            } catch (e: Exception) {
                                Utils.handleException(e, "processSnapshotOpenResult: addOnCompleteListener")
                            }
                        }
                }
            }
        }
        catch(e: Exception) {
            Utils.handleException(e, "loadSnapshot")
        }
    }

    fun signInSilently() {
        try {
            Log.d(Utils.TAG, "signInSilently")
            gamesSignInClient.isAuthenticated()
                .addOnCompleteListener { authTask: Task<AuthenticationResult?>? ->
                    try {
                        if (isAuthenticated(authTask)) {
                            // Continue with Play Games Services
                            onConnected()
                        } else {
                            // If authentication fails, either disable Play Games Services
                            // integration or
                            // display a login button to prompt players to sign in.
                            // Use`gamesSignInClient.signIn()` when the login button is clicked.
                            onDisconnected()
                        }
                    }
                    catch(e: Exception) {
                        Utils.handleException(e, "signInSilently: addOnCompleteListener")
                    }
                }
        }
        catch(e: Exception) {
            Utils.handleException(e, "signInSilently")
        }
    }

    fun startSignInIntent() {
        try {
            Log.d(Utils.TAG, "startSignInIntent")
            gamesSignInClient
                .signIn()
                .addOnCompleteListener { authTask: Task<AuthenticationResult?>? ->
                    try {
                        if (isAuthenticated(authTask)) {
                            // sign in successful
                            onConnected()
                        } else {
                            // sign in failed
                            onDisconnected()
                        }
                    }
                    catch(e: Exception) {
                        Utils.handleException(e, "startSignInIntent: addOnCompleteListener")
                    }
                }
        }
        catch(e: Exception) {
            Utils.handleException(e, "startSignInIntent")
        }
    }

    fun submitScore(leaderboardId: String, value: Long) {
        try {
            Log.d(Utils.TAG, "submitScore $leaderboardId: $value")
            getLeaderboardsClient().submitScore(leaderboardId, value)
        }
        catch(e: Exception) {
            Utils.handleException(e, "submitScore")
        }
    }

    fun showLeaderboards() {
        try {
            Log.d(Utils.TAG, "showLeaderboards")
            Utils.logFirebasePlayServicesAction("show_leaderboards", null)
            getLeaderboardsClient().allLeaderboardsIntent.addOnSuccessListener { intent ->
                try {
                    activity.startActivityForResult(intent, RC_UNUSED)
                }
                catch(e: Exception) {
                    Utils.handleException(e, "showLeaderboards: addOnSuccessListener")
                }
            }
        }
        catch(e: Exception) {
            Utils.handleException(e, "showLeaderboards")
        }
    }

    fun showAchievements() {
        try {
            Log.d(Utils.TAG, "showAchievements")
            Utils.logFirebasePlayServicesAction("show_achievements", null)
            getAchievementsClient().achievementsIntent.addOnSuccessListener { intent ->
                try {
                    activity.startActivityForResult(intent, RC_UNUSED)
                }
                catch(e: Exception) {
                    Utils.handleException(e, "showAchievements: addOnSuccessListener")
                }
            }
        }
        catch(e: Exception) {
            Utils.handleException(e, "showAchievements")
        }
    }

    fun incrementAchievement(code: String, value: Int) {
        try {
            Log.d(Utils.TAG, "incrementAchievement $code=$value")
            getAchievementsClient().increment(code, value)
        }
        catch(e: Exception) {
            Utils.handleException(e, "incrementAchievement")
        }
    }

    fun revealAchievement(code: String) {
        try {
            Log.d(Utils.TAG, "revealAchievement $code")
            getAchievementsClient().reveal(code)
        }
        catch(e: Exception) {
            Utils.handleException(e, "revealAchievement")
        }
    }

    fun achievementSetSteps(code: String, value: Int) {
        try {
            Log.d(Utils.TAG, "achievementSetSteps $code=$value")
            getAchievementsClient().setSteps(code, value)
        }
        catch(e: Exception) {
            Utils.handleException(e, "achievementSetSteps")
        }
    }

    fun unlockAchievement(code: String) {
        try {
            Log.d(Utils.TAG, "unlockAchievement $code")
            Utils.logFirebasePlayServicesAction("unlock_achievement", code)
            getAchievementsClient().unlock(code)
        }
        catch(e: Exception) {
            Utils.handleException(e, "unlockAchievement")
        }
    }

    fun deleteSnapshot() {

    }

    fun showSnapshots() {

    }

    fun isAuthenticated(): Boolean = connected

    fun saveSnapshot(data: ByteArray) {
        Log.d(Utils.TAG, "saveSnapshot: size=${data.size}")
        waitForClosedAndOpen().addOnCompleteListener { task ->
            if (!task.isSuccessful) {
                Utils.handleException(task.exception, "saveSnapshot: waitForClosedAndOpen failed")
                return@addOnCompleteListener
            }
            task.result?.let { result ->
                processSnapshotOpenResult(result, 0)
                    .addOnCompleteListener(object : OnCompleteListener<Snapshot?> {
                        override fun onComplete(task: Task<Snapshot?>) {
                            try {
                                if (!task.isSuccessful) {
                                    Utils.handleException(task.exception, "saveSnapshot: processSnapshotOpenResult failed")
                                    return
                                }

                                val snapshotToWrite =
                                    task.getResult() ?: // No snapshot available yet; waiting on the user to choose one.
                                    return

                                Log.d(
                                    Utils.TAG,
                                    "Writing data to snapshot: " + snapshotToWrite.metadata.uniqueName
                                )
                                writeSnapshot(snapshotToWrite, data)
                                    ?.addOnCompleteListener { task ->
                                        try {
                                            if (task.isSuccessful) {
                                                Log.i(Utils.TAG, "Snapshot saved. Size: " + data.size)
                                            } else {
                                                Utils.handleException(task.exception, "write_snapshot_error")
                                            }
                                        } catch (e: java.lang.Exception) {
                                            Utils.handleException(e, "writeSnapshot: addOnCompleteListener")
                                        }
                                    }
                            } catch (e: java.lang.Exception) {
                                Utils.handleException(e, "saveSnapshot")
                            }
                        }
                    })
            }
        }
    }

    fun loadLeaderboardScoreAsync(leaderboardId: String, callback: (Long) -> Boolean) {
        getLeaderboardsClient().loadCurrentPlayerLeaderboardScore(
            leaderboardId,
            LeaderboardVariant.TIME_SPAN_ALL_TIME,
            LeaderboardVariant.COLLECTION_PUBLIC).addOnCompleteListener { task ->
                if(task.isSuccessful) {
                    var score = 0L
                    try {
                        score = task.getResult().get()?.rawScore ?: 0L
                    }
                    catch(e: Exception) {
                        Log.d(Utils.TAG, e.toString())
                    }
                    Log.d(Utils.TAG, "Loaded score for $leaderboardId = $score")
                    val result = callback(score)
                    Log.d(Utils.TAG, "Result of loadLeaderboardScoreAsync callback $result")
                }
        }
    }

    private fun writeSnapshot(snapshot: Snapshot, data: ByteArray): Task<SnapshotMetadata?>? {
        // Set the data payload for the snapshot.
        try {
            snapshot.snapshotContents.writeBytes(data)

            // Save the snapshot.
            val metadataChange = SnapshotMetadataChange.Builder()
                .setDescription("Modified data at: ")
                .build()

            return SnapshotCoordinator.getInstance().commitAndClose(getSnapshotsClient(), snapshot, metadataChange)
        }
        catch(e: Exception) {
            Utils.handleException(e, "writeSnapshot")
        }
        return null
    }


    private fun processSnapshotOpenResult(result: DataOrConflict<Snapshot?>, retryCount: Int): Task<Snapshot?> {
        if (!result.isConflict) {
            // There was no conflict, so return the result of the source.
            val source = TaskCompletionSource<Snapshot?>()
            source.setResult(result.getData())
            return source.getTask()
        }

        val conflict = result.conflict

        // always take a server version
        val resolvedSnapshot = conflict!!.snapshot

        return getSnapshotsClient()
            .resolveConflict(conflict.conflictId, resolvedSnapshot)
            .continueWithTask<Snapshot?>(
                object : Continuation<DataOrConflict<Snapshot?>?, Task<Snapshot?>?> {
                    @Throws(java.lang.Exception::class)
                    override fun then(
                        task: Task<DataOrConflict<Snapshot?>?>
                    ): Task<Snapshot?> {
                        if (!task.isSuccessful) {
                            // SNAPSHOT_CONFLICT_MISSING (26576): the conflict was already resolved
                            // elsewhere (another device/session). Treat as no data — return null
                            // so the caller skips reading and proceeds to close.
                            Log.w(Utils.TAG, "resolveConflict failed (conflict may be gone): ${task.exception?.message}")
                            // IMPORTANT: resolveConflict was called directly on GMS (not through
                            // SnapshotCoordinator), so the coordinator still considers the file
                            // "opened". We must call discardAndClose to release the CountDownLatch;
                            // otherwise the next waitForClosedAndOpen() will deadlock.
                            SnapshotCoordinator.getInstance()
                                .discardAndClose(getSnapshotsClient(), resolvedSnapshot)
                                .addOnFailureListener { e ->
                                    Utils.handleException(e, "resolveConflict: discardAndClose failed")
                                }
                            val source = TaskCompletionSource<Snapshot?>()
                            source.setResult(null)
                            return source.getTask()
                        }
                        // Resolving the conflict may cause another conflict,
                        // so recurse and try another resolution.
                        if (retryCount < MAX_SNAPSHOT_RESOLVE_RETRIES) {
                            return processSnapshotOpenResult(task.getResult()!!, retryCount + 1)
                        } else {
                            throw java.lang.Exception("Could not resolve snapshot conflicts")
                        }
                    }
                })
    }

    private fun waitForClosedAndOpen(): Task<DataOrConflict<Snapshot?>?> =
        SnapshotCoordinator.getInstance()
            .waitForClosed(SAVED_GAME)
            .addOnFailureListener { e -> Utils.handleException(e, "There was a problem waiting for the file to close!") }
            .continueWithTask<DataOrConflict<Snapshot?>?> {
                val openTask =
                    SnapshotCoordinator.getInstance().open(getSnapshotsClient(), SAVED_GAME, true)
                openTask.addOnFailureListener { e -> Utils.handleException(e, "error_opening_filename") }
            }

    private fun isAuthenticated(authTask: Task<AuthenticationResult?>?) =
        authTask?.isSuccessful == true && authTask.getResult()?.isAuthenticated == true

    private fun onDisconnected() {
        connected = false
        Log.d(Utils.TAG, "onDisconnected")
        try {
            playServicesObserver.onPlayServicesDisconnected()
        } catch(e: Exception) {
            Utils.handleException(e, "onDisconnected")
        } catch(e: UnsatisfiedLinkError) {
            Utils.handleException(e, "onDisconnected")
        }
    }

    private fun onConnected() {
        connected = true
        Log.d(Utils.TAG, "onConnected")
        try {
            playServicesObserver.onPlayServicesConnected()
        } catch(e: Exception) {
            Utils.handleException(e, "onConnected")
        } catch(e: UnsatisfiedLinkError) {
            Utils.handleException(e, "onConnected")
        }

        if(useSnapshot && !initialSnapshotLoadStarted) {
            initialSnapshotLoadStarted = true
            try {
                loadSnapshot()
            }
            catch (e: Exception) {
                Log.e(Utils.TAG, e.toString())
            }
        }
    }

    private fun onSnapshotReceived(array: ByteArray) {
        Log.d(Utils.TAG, "onSnapshotReceived: size=${array.size}")
        try {
            playServicesObserver.onSnapshotReceived(array, array.size)
        } catch(e: Exception) {
            Utils.handleException(e, "onSnapshotReceived")
        } catch(e: UnsatisfiedLinkError) {
            Utils.handleException(e, "onSnapshotReceived")
        }
    }

    private fun getSnapshotsClient() = PlayGames.getSnapshotsClient(activity)
    private fun getLeaderboardsClient() = PlayGames.getLeaderboardsClient(activity)
    private fun getAchievementsClient() = PlayGames.getAchievementsClient(activity);

    private companion object {
        const val SAVED_GAME = "default_saved_game"
        const val RC_UNUSED = 5001
        const val MAX_SNAPSHOT_RESOLVE_RETRIES = 40
    }
}