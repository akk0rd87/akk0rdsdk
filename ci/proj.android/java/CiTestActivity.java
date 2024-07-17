package org.popapp.citest;

import org.akkord.lib.AkkordActivity;

import android.app.PendingIntent;
import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.core.content.FileProvider;

import android.util.Log;

import com.google.android.gms.games.LeaderboardsClient;
import com.google.android.gms.auth.api.signin.GoogleSignIn;
import com.google.android.gms.auth.api.signin.GoogleSignInAccount;
import com.google.android.gms.auth.api.signin.GoogleSignInClient;
import com.google.android.gms.auth.api.signin.GoogleSignInOptions;
import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.games.Games;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.gms.tasks.Task;

import com.google.android.gms.drive.Drive;
import com.google.android.gms.games.SnapshotsClient;
import com.google.android.gms.games.snapshot.Snapshot;
import com.google.android.gms.games.snapshot.SnapshotMetadata;
import com.google.android.gms.games.snapshot.SnapshotMetadataChange;

import com.google.android.gms.tasks.Continuation;
import android.content.pm.ActivityInfo;
import java.io.File;
import java.util.Objects;

import org.akkord.lib.SnapshotCoordinator;
import org.popapp.citest.MyBroadcastReceiver;

import com.google.android.gms.common.api.Result;
import com.google.android.gms.tasks.TaskCompletionSource;
//import com.google.firebase.analytics.FirebaseAnalytics;

import static android.app.PendingIntent.FLAG_IMMUTABLE;

public class CiTestActivity extends AkkordActivity
{
    final static String TAG = "SDL";

    private long last_XSmall  = 0;
    private long last_Small   = 0;
    private long last_Medium  = 0;
    private long last_Large   = 0;
    private long last_XLarge  = 0;

    private static final String SavedGameName = "default_saved_game";

    // Event codes for native Callback
    private static final int NATIVE_SIGN_IN              = 1;
    private static final int NATIVE_SIGN_OUT             = 2;

    private final static int RC_UNUSED = 5001;
    // Request code used to invoke sign in user interactions.
    private static final int RC_SIGN_IN = 9001;

    // Request code for listing saved games
    private static final int RC_LIST_SAVED_GAMES = 9002;

    private static final int MAX_SNAPSHOT_RESOLVE_RETRIES = 40;

    // native functions
    public static native void PlayServicesCallback(int EventCode, int Data1, int Data2);
    public static native void SnapshotReceivedCallback(byte[] Array, int Size, int WasConflict);

    //private static FirebaseAnalytics mFirebaseAnalytics = null;
    // Client used to sign in with Google APIs
    private GoogleSignInClient mGoogleSignInClient = null;

    // Client used to interact with Google Snapshots.
    private SnapshotsClient mSnapshotsClient = null;

    // The currently signed in account, used to check the account has changed outside of this activity when resuming.
    GoogleSignInAccount mSignedInAccount = null;

    private LeaderboardsClient mLeaderboardsClient = null;

    private boolean InitialSnapshotLoadStarted = false;

    private void initGoogleSignInClient() {
        try {
            // Create the client used to sign in.
            mGoogleSignInClient = GoogleSignIn.getClient(this,
                    new GoogleSignInOptions.Builder(GoogleSignInOptions.DEFAULT_GAMES_SIGN_IN)
                            // Since we are using SavedGames, we need to add the SCOPE_APPFOLDER to access Google Drive.
                            .requestScopes(Drive.SCOPE_APPFOLDER)
                            .requestScopes(Games.SCOPE_GAMES_LITE)
                            .build());
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(null);
        initGoogleSignInClient();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume CiTestActivity()");

        // Since the state of the signed in user can change when the activity is not active
        // it is recommended to try and sign in silently from when the app resumes.
        signInSilently();
    }

    private void onDisconnected() {
        try{
            Log.d(TAG, "onDisconnected()");
            mLeaderboardsClient = null;
            mSnapshotsClient = null;
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    private void onConnected(GoogleSignInAccount googleSignInAccount) {
        try {
            Log.d(TAG, "onConnected(): connected to Google APIs");
            if (mSignedInAccount != googleSignInAccount) {
                mSignedInAccount = googleSignInAccount;
            }
            mLeaderboardsClient = Games.getLeaderboardsClient(this, googleSignInAccount);
            mSnapshotsClient = Games.getSnapshotsClient(this, googleSignInAccount);

            if(!InitialSnapshotLoadStarted)
            {
                InitialSnapshotLoadStarted = true;
                LoadSnapshot();
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    @Override
    public void setOrientationBis(int w, int h, boolean resizable, String hint) {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_USER);
    }

    private void signInSilently() {
        Log.d(TAG, "signInSilently()");

        try {
            if (mGoogleSignInClient != null) {
                mGoogleSignInClient.silentSignIn().addOnCompleteListener(this,
                        new OnCompleteListener<GoogleSignInAccount>() {
                            @Override
                            public void onComplete(@NonNull Task<GoogleSignInAccount> task) {
                                try {
                                    if (task.isSuccessful()) {
                                        Log.d(TAG, "signInSilently(): success");
                                        PlayServicesCallback(NATIVE_SIGN_IN, 0, 0);
                                        onConnected(task.getResult());

                                    } else {
                                        Log.d(TAG, "signInSilently(): failure", task.getException());
                                        PlayServicesCallback(NATIVE_SIGN_OUT, 0, 0);
                                        onDisconnected();
                                    }
                                }
                                catch(Exception e) {
                                    Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                                }
                            }
                        });
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);

        try {
            if (requestCode == RC_SIGN_IN) {
                Task<GoogleSignInAccount> task =
                        GoogleSignIn.getSignedInAccountFromIntent(intent);

                try {
                    GoogleSignInAccount account = task.getResult(ApiException.class);
                    onConnected(account);
                    PlayServicesCallback(NATIVE_SIGN_IN, 0, 0);
                } catch (ApiException apiException) {
                    String message = apiException.getMessage();
                    if (message == null || message.isEmpty()) {
                        message = "signin_other_error";
                    }

                    onDisconnected();
                    PlayServicesCallback(NATIVE_SIGN_OUT, 0, 0);

                    /*
                    new AlertDialog.Builder(this)
                            .setMessage(message)
                            .setNeutralButton(android.R.string.ok, null)
                            .show();
                    */
                }
            }

            else if (requestCode == RC_LIST_SAVED_GAMES) {
                // the standard snapshot selection intent
                /*
                if (intent != null) {
                    if (intent.hasExtra(SnapshotsClient.EXTRA_SNAPSHOT_METADATA)) {
                        // Load a snapshot.
                        SnapshotMetadata snapshotMetadata =
                                intent.getParcelableExtra(SnapshotsClient.EXTRA_SNAPSHOT_METADATA);
                        currentSaveName = snapshotMetadata.getUniqueName();
                        loadFromSnapshot(snapshotMetadata);
                    } else if (intent.hasExtra(SnapshotsClient.EXTRA_SNAPSHOT_NEW)) {
                        // Create a new snapshot named with a unique string
                        String unique = Long.toString(System.currentTimeMillis());
                        currentSaveName = "snapshotTemp-" + unique;
                        saveSnapshot(null);
                    }
                }
                */
            }
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public void SignIn() {
        try {
            if (mGoogleSignInClient != null) {
                startActivityForResult(mGoogleSignInClient.getSignInIntent(), RC_SIGN_IN);
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public void SignOut() {
        Log.d(TAG, "signOut()");

        try {
            if (mGoogleSignInClient != null) {
                mGoogleSignInClient.signOut().addOnCompleteListener(this,
                        new OnCompleteListener<Void>() {
                            @Override
                            public void onComplete(@NonNull Task<Void> task) {
                                try {
                                    if (task.isSuccessful()) {
                                        Log.d(TAG, "signOut(): success");
                                        InitialSnapshotLoadStarted = false;
                                        PlayServicesCallback(NATIVE_SIGN_OUT, 0, 0);
                                    } else {
                                        Log.e(TAG, "signOut(): failed");
                                        //handleException(task.getException(), "signOut() failed!");
                                    }
                                    onDisconnected();
                                }
                                catch(Exception e) {
                                    Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                                }
                            }
                        });
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public void SubmitScore(long XSmall, long Small, long Medium, long Large, long XLarge)
    {
        try {
            if (mLeaderboardsClient != null) {
                boolean submitScore = false;
                if(last_XSmall != XSmall) {
                    mLeaderboardsClient.submitScore("000000000000000000", XSmall);
                    submitScore = true;
                }

                if(last_Small != Small) {
                    mLeaderboardsClient.submitScore("000000000000000000", Small);
                    submitScore = true;
                }

                if(last_Medium != Medium) {
                    mLeaderboardsClient.submitScore("000000000000000000", Medium);
                    submitScore = true;
                }

                if(last_Large != Large) {
                    mLeaderboardsClient.submitScore("000000000000000000", Large);
                    submitScore = true;
                }

                if(last_XLarge != XLarge) {
                    mLeaderboardsClient.submitScore("000000000000000000", XLarge);
                    submitScore = true;
                }

                last_XSmall  = XSmall;
                last_Small   = Small ;
                last_Medium  = Medium;
                last_Large   = Large ;
                last_XLarge  = XLarge;

                /*
                if(submitScore && mFirebaseAnalytics != null) {
                    try {
                        Bundle params = new Bundle();
                        params.putString("XSmall", Long.toString(XSmall));
                        params.putString("Small", Long.toString(Small));
                        params.putString("Medium", Long.toString(Medium));
                        params.putString("Large", Long.toString(Large));
                        params.putString("XLarge", Long.toString(XLarge));
                        mFirebaseAnalytics.logEvent(ANALYTIC_EVENT_SUBMIT_SCORE, params);
                    } catch (Exception e) {
                        Log.e(TAG, "Error while log event: " + e.getMessage());
                    }
                }
                */
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public void ShowLeaderboards()
    {
        try {
            if(mLeaderboardsClient != null) {
                mLeaderboardsClient.getAllLeaderboardsIntent()
                        .addOnSuccessListener(new OnSuccessListener<Intent>() {
                            @Override
                            public void onSuccess(Intent intent) {
                                try {
                                    startActivityForResult(intent, RC_UNUSED);
                                }
                                catch(Exception e) {
                                    Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                                }
                            }
                        });
                        //.addOnFailureListener(new OnFailureListener() {
                        //    @Override
                        //    public void onFailure(@NonNull Exception e) {
                        //        Log.e(TAG, "ShowLeaderboards exception");
                        //    }
                        //});
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }


    /////////////// Saved games https://developers.google.com/games/services/android/savedgames
    // for automatic conflict resolving
    private Task<Snapshot> processSnapshotOpenResult(SnapshotsClient.DataOrConflict<Snapshot> result, final int retryCount) {
        if (!result.isConflict()) {
            // There was no conflict, so return the result of the source.
            TaskCompletionSource<Snapshot> source = new TaskCompletionSource<>();
            source.setResult(result.getData());
            return source.getTask();
        }

        SnapshotsClient.SnapshotConflict conflict = result.getConflict();

        // always take a server version
        Snapshot resolvedSnapshot = conflict.getSnapshot();

        return Games.getSnapshotsClient(this, GoogleSignIn.getLastSignedInAccount(this))
                .resolveConflict(conflict.getConflictId(), resolvedSnapshot)
                .continueWithTask(
                        new Continuation<
                                SnapshotsClient.DataOrConflict<Snapshot>,
                                Task<Snapshot>>() {
                            @Override
                            public Task<Snapshot> then(
                                    @NonNull Task<SnapshotsClient.DataOrConflict<Snapshot>> task)
                                    throws Exception {
                                // Resolving the conflict may cause another conflict,
                                // so recurse and try another resolution.
                                if (retryCount < MAX_SNAPSHOT_RESOLVE_RETRIES) {
                                    return processSnapshotOpenResult(task.getResult(), retryCount + 1);
                                } else {
                                    throw new Exception("Could not resolve snapshot conflicts");
                                }
                            }
                        });
    }

    private void handleException(Exception exception, String details) {
        Log.e(TAG, exception.getMessage() + "; Details: " + details);
    }

    private Task<SnapshotsClient.DataOrConflict<Snapshot>> waitForClosedAndOpen() {
        return SnapshotCoordinator.getInstance()
                .waitForClosed(SavedGameName)
                .addOnFailureListener(new OnFailureListener() {
                    @Override
                    public void onFailure(@NonNull Exception e) {
                        handleException(e, "There was a problem waiting for the file to close!");
                    }
                })
                .continueWithTask(new Continuation<Result, Task<SnapshotsClient.DataOrConflict<Snapshot>>>() {
                    @Override
                    public Task<SnapshotsClient.DataOrConflict<Snapshot>> then(@NonNull Task<Result> task) throws Exception {
                        Task<SnapshotsClient.DataOrConflict<Snapshot>> openTask = SnapshotCoordinator.getInstance().open(mSnapshotsClient, SavedGameName, true);
                        return openTask.addOnFailureListener(new OnFailureListener() {
                            @Override
                            public void onFailure(@NonNull Exception e) {
                                handleException(e, "error_opening_filename");
                            }
                        });
                    }
                });
    }

    private Task<SnapshotMetadata> writeSnapshot(Snapshot snapshot, final byte[] Data) {
        // Set the data payload for the snapshot.
        snapshot.getSnapshotContents().writeBytes(Data);

        // Save the snapshot.
        SnapshotMetadataChange metadataChange = new SnapshotMetadataChange.Builder()
                .setDescription("Modified data at: ")
                .build();

        return SnapshotCoordinator.getInstance().commitAndClose(mSnapshotsClient, snapshot, metadataChange);
    }


    private void SaveSnapshot_internal(final byte[] Data)
    {
        if(mSnapshotsClient == null)
        {
            return;
        }

        waitForClosedAndOpen()
                .addOnCompleteListener(new OnCompleteListener<SnapshotsClient.DataOrConflict<Snapshot>>() {
                    @Override
                    public void onComplete(@NonNull Task<SnapshotsClient.DataOrConflict<Snapshot>> task) {
                        try {
                            SnapshotsClient.DataOrConflict<Snapshot> result = task.getResult();

                            processSnapshotOpenResult(result, 0)
                                    .addOnCompleteListener(new OnCompleteListener<Snapshot>(){
                                        @Override
                                        public void onComplete(@NonNull Task<Snapshot> task) {
                                            try {
                                                Snapshot snapshotToWrite = task.getResult();
                                                if (snapshotToWrite == null) {
                                                    // No snapshot available yet; waiting on the user to choose one.
                                                    return;
                                                }

                                                Log.d(TAG, "Writing data to snapshot: " + snapshotToWrite.getMetadata().getUniqueName());
                                                writeSnapshot(snapshotToWrite, Data)
                                                        .addOnCompleteListener(new OnCompleteListener<SnapshotMetadata>() {
                                                            @Override
                                                            public void onComplete(@NonNull Task<SnapshotMetadata> task) {
                                                                try {
                                                                    if (task.isSuccessful()) {
                                                                        Log.i(TAG, "Snapshot saved. Size: " + Data.length);
                                                                    } else {
                                                                        handleException(Objects.requireNonNull(task.getException()), "write_snapshot_error");
                                                                    }
                                                                }
                                                                catch(Exception e) {
                                                                    Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                                                                }
                                                            }
                                                        });
                                            }
                                            catch(Exception e) {
                                                Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                                            }
                                        }
                                    });
                        }
                        catch(Exception e) {
                            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                        }
                    }
                });
    }

    private void LoadSnapshot_internal()
    {
        if(mSnapshotsClient == null)
        {
            return;
        }

        waitForClosedAndOpen()
                .addOnSuccessListener(new OnSuccessListener<SnapshotsClient.DataOrConflict<Snapshot>>() {
                    @Override
                    public void onSuccess(SnapshotsClient.DataOrConflict<Snapshot> result) {

                        // если был конфликт, мы должны после обработки в плюсах отправить новую версию
                        final int WasConflict = (result.isConflict() ? 1 : 0);

                        if(WasConflict > 0)
                        {
                            Log.i(TAG, "IS CONFLICT !!!");
                        }

                        processSnapshotOpenResult(result, 0)
                                .addOnCompleteListener(new OnCompleteListener<Snapshot>(){
                                    @Override
                                    public void onComplete(@NonNull Task<Snapshot> task) {
                                        try {
                                            Snapshot snapshot = task.getResult();

                                            if (snapshot == null) {
                                                Log.w(TAG, "Conflict was not resolved automatically, waiting for user to resolve.");
                                            } else {
                                                try {
                                                    //readSavedGame(snapshot);
                                                    final byte[] Data =  snapshot.getSnapshotContents().readFully();
                                                    Log.i(TAG, "Snapshot loaded. Size: " + Data.length);
                                                    SnapshotReceivedCallback(Data, Data.length, WasConflict);
                                                } catch (/*IOException*/ Exception e) {
                                                    Log.e(TAG, "Error while reading snapshot contents: " + e.getMessage());
                                                }
                                            }

                                            SnapshotCoordinator.getInstance().discardAndClose(mSnapshotsClient, snapshot)
                                                    .addOnFailureListener(new OnFailureListener() {
                                                        @Override
                                                        public void onFailure(@NonNull Exception e) {
                                                            handleException(e, "There was a problem discarding the snapshot!");
                                                        }
                                                    });
                                        }
                                        catch(Exception e) {
                                            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                                        }
                                    }
                                });
                    }
                });
    }

    public void SaveSnapshot(final byte[] Data)
    {
        try {
            Thread t = new Thread(new Runnable() { public void run() {
                try {
                    SaveSnapshot_internal(Data);
                }
                catch(Exception e) {
                    Log.e(TAG, "Error while SaveSnapshot: " + e.getMessage());
                }
            }});
            t.start();
        }
        catch(Exception e)
        {
            Log.e(TAG, "Error while SaveSnapshot: " + e.getMessage());
        }
    }

    public void LoadSnapshot()
    {
        try {
            LoadSnapshot_internal();
        }
        catch(Exception e)
        {
            Log.e(TAG, "Error while LoadSnapshot: " + e.getMessage());
        }
    }

    public void DeleteSnapshot()
    {

    }

    public void ShowSnapshots()
    {
        try {
            Log.d(TAG, "ShowSnapshots");
            int maxNumberOfSavedGamesToShow = 5;
            SnapshotCoordinator.getInstance().getSelectSnapshotIntent(
                    mSnapshotsClient, "Snapshots", false, true, maxNumberOfSavedGamesToShow)
                    .addOnCompleteListener(new OnCompleteListener<Intent>() {
                        @Override
                        public void onComplete(@NonNull Task<Intent> task) {
                            try {
                                if (task.isSuccessful()) {
                                    startActivityForResult(task.getResult(), RC_LIST_SAVED_GAMES);
                                } else {
                                    handleException(Objects.requireNonNull(task.getException()), "show_snapshots_error");
                                }
                            }
                            catch(Exception e) {
                                Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                            }
                        }
                    });
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    private void ShareFile(final String Title, final String Fname, final String MimeType) {
        try {
            if (android.os.Build.VERSION.SDK_INT >= 14)
            {
                class OneShotTask implements Runnable {
                    OneShotTask() {}

                    public void run() {
                        try {
                            Log.v(TAG, "ShareFile started: " + Fname);
                            Intent sendIntent = new Intent();
                            sendIntent.setAction(Intent.ACTION_SEND);
                            sendIntent.setType(MimeType);
                            sendIntent.putExtra(Intent.EXTRA_STREAM,
                                    FileProvider.getUriForFile(
                                            getContext(),
                                            getPackageName() + ".provider",
                                            new File(Fname))
                            );

                            if (android.os.Build.VERSION.SDK_INT >= 22) {
                                PendingIntent pi = PendingIntent.getBroadcast(getContext(), 0,
                                        new Intent(getContext(), org.popapp.citest.MyBroadcastReceiver.class),
                                        FLAG_IMMUTABLE);
                                startActivity(Intent.createChooser(sendIntent, Title, pi.getIntentSender()));
                            }
                            else {
                                startActivity(Intent.createChooser(sendIntent, Title));
                            }
                        } catch (Exception e) {
                            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                        }
                    }
                };
                runOnUiThread(new OneShotTask());
            }
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public void ShareImage(final String Title, final String File) {
        ShareFile(Title, File, "image/png");
    }

    public void SharePDF(final String Title, final String File) {
        ShareFile(Title, File, "application/pdf");
    }

    public static void logImageSendEvent(final String Component) {
        /*
        try {
            if(mFirebaseAnalytics != null) {
                Bundle params = new Bundle();
                params.putString("activity", Component);
                mFirebaseAnalytics.logEvent(ANALYTIC_EVENT_SHARE_IMAGE_CHOOSER_SELECT_ACTIVITY, params);
            }
        }
        catch(Exception e) {
            Log.e(TAG, "Error while log event: " + e.getMessage());
        }
        */
    }
}
