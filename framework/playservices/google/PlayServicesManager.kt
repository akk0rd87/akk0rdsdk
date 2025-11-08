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

private external fun playServicesCallback(eventCode: Int, data1: Int, data2: Int)
private external fun snapshotReceivedCallback(array: ByteArray?, size: Int, wasConflict: Int)

class PlayServicesManager(
    private val activity: Activity,
    private val useSnapshot: Boolean,
) {
    init {
        PlayGamesSdk.initialize(activity)
    }

    private val gamesSignInClient: GamesSignInClient = PlayGames.getGamesSignInClient(activity)
    private var initialSnapshotLoadStarted = false
    private var connected = false

    fun loadSnapshot() {
        try {
            Log.d(Utils.TAG, "loadSnapshot")
            waitForClosedAndOpen().addOnSuccessListener { result ->
                if (result != null) {
                    val wasConflict = (if (result.isConflict) 1 else 0)

                    if (wasConflict > 0) {
                        Log.i(Utils.TAG, "IS CONFLICT !!!")
                    }

                    processSnapshotOpenResult(result, 0)
                        .addOnCompleteListener { task ->
                            try {
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
                                        snapshotReceivedCallback(data, data.size, wasConflict)
                                    } catch (e: Exception) {
                                        Log.e(Utils.TAG, "Error while reading snapshot contents: " + e.message)
                                    }
                                }

                                SnapshotCoordinator.getInstance().discardAndClose(getSnapshotsClient(), snapshot)
                                    .addOnFailureListener { e ->
                                        handleException(e, "There was a problem discarding the snapshot!")
                                    }
                            } catch (e: Exception) {
                                handleException(e, "processSnapshotOpenResult: addOnCompleteListener")
                            }
                        }
                }
            }
        }
        catch(e: Exception) {
            handleException(e, "loadSnapshot")
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
                        handleException(e, "signInSilently: addOnCompleteListener")
                    }
                }
        }
        catch(e: Exception) {
            handleException(e, "signInSilently")
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
                        handleException(e, "startSignInIntent: addOnCompleteListener")
                    }
                }
        }
        catch(e: Exception) {
            handleException(e, "startSignInIntent")
        }
    }

    fun submitScore(leaderboardId: String, value: Long) {
        try {
            Log.d(Utils.TAG, "submitScore $leaderboardId: $value")
            getLeaderboardsClient().submitScore(leaderboardId, value)
        }
        catch(e: Exception) {
            handleException(e, "submitScore")
        }
    }

    fun showLeaderboards() {
        try {
            Log.d(Utils.TAG, "showLeaderboards")
            getLeaderboardsClient().allLeaderboardsIntent.addOnSuccessListener { intent ->
                try {
                    activity.startActivityForResult(intent, RC_UNUSED)
                }
                catch(e: Exception) {
                    handleException(e, "showLeaderboards: addOnSuccessListener")
                }
            }
        }
        catch(e: Exception) {
            handleException(e, "showLeaderboards")
        }
    }

    fun showAchievements() {
        try {
            Log.d(Utils.TAG, "showAchievements")
            getAchievementsClient().achievementsIntent.addOnSuccessListener { intent ->
                try {
                    activity.startActivityForResult(intent, RC_UNUSED)
                }
                catch(e: Exception) {
                    handleException(e, "showAchievements: addOnSuccessListener")
                }
            }
        }
        catch(e: Exception) {
            handleException(e, "showAchievements")
        }
    }

    fun incrementAchievement(code: String, value: Int) {
        try {
            Log.d(Utils.TAG, "incrementAchievement $code=$value")
            getAchievementsClient().increment(code, value)
        }
        catch(e: Exception) {
            handleException(e, "incrementAchievement")
        }
    }

    fun revealAchievement(code: String) {
        try {
            Log.d(Utils.TAG, "revealAchievement $code")
            getAchievementsClient().reveal(code)
        }
        catch(e: Exception) {
            handleException(e, "revealAchievement")
        }
    }

    fun achievementSetSteps(code: String, value: Int) {
        try {
            Log.d(Utils.TAG, "achievementSetSteps $code=$value")
            getAchievementsClient().setSteps(code, value)
        }
        catch(e: Exception) {
            handleException(e, "achievementSetSteps")
        }
    }

    fun unlockAchievement(code: String) {
        try {
            Log.d(Utils.TAG, "unlockAchievement $code")
            getAchievementsClient().unlock(code)
        }
        catch(e: Exception) {
            handleException(e, "unlockAchievement")
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
            task.result?.let { result ->
                processSnapshotOpenResult(result, 0)
                    .addOnCompleteListener(object : OnCompleteListener<Snapshot?> {
                        override fun onComplete(task: Task<Snapshot?>) {
                            try {
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
                                                handleException(task.exception, "write_snapshot_error")
                                            }
                                        } catch (e: java.lang.Exception) {
                                            handleException(e, "writeSnapshot: addOnCompleteListener")
                                        }
                                    }
                            } catch (e: java.lang.Exception) {
                                handleException(e, "saveSnapshot")
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
                val score  = task.getResult().get()?.rawScore ?: 0L
                Log.d(Utils.TAG, "Loaded score for $leaderboardId = $score")
                val result = callback(score)
                Log.d(Utils.TAG, "Result of loadLeaderboardScoreAsync callback $result")
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
            handleException(e, "writeSnapshot")
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
            .addOnFailureListener { e -> handleException(e, "There was a problem waiting for the file to close!") }
            .continueWithTask<DataOrConflict<Snapshot?>?> {
                val openTask =
                    SnapshotCoordinator.getInstance().open(getSnapshotsClient(), SAVED_GAME, true)
                openTask.addOnFailureListener { e -> handleException(e, "error_opening_filename") }
            }

    private fun handleException(exception: java.lang.Exception?, details: String?) {
        Log.e(Utils.TAG, exception?.message + "; Details: " + details)
    }

    private fun isAuthenticated(authTask: Task<AuthenticationResult?>?) =
        authTask?.isSuccessful == true && authTask.getResult()?.isAuthenticated == true

    private fun onDisconnected() {
        connected = false
        Log.d(Utils.TAG, "onDisconnected")
        playServicesCallback(NATIVE_SIGN_OUT, 0, 0);
    }

    private fun onConnected() {
        connected = true
        Log.d(Utils.TAG, "onConnected")
        playServicesCallback(NATIVE_SIGN_IN, 0, 0)

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

    private fun getSnapshotsClient() = PlayGames.getSnapshotsClient(activity)
    private fun getLeaderboardsClient() = PlayGames.getLeaderboardsClient(activity)
    private fun getAchievementsClient() = PlayGames.getAchievementsClient(activity);

    private companion object {
        const val SAVED_GAME = "default_saved_game"
        const val NATIVE_SIGN_IN: Int = 1
        const val NATIVE_SIGN_OUT: Int = 2
        const val RC_UNUSED = 5001
        const val MAX_SNAPSHOT_RESOLVE_RETRIES = 40
    }
}