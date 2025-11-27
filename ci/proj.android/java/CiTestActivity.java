package org.popapp.citest;

import org.akkord.lib.AkkordActivity;

import android.app.PendingIntent;
import android.content.Intent;
import android.os.Bundle;
import androidx.core.content.FileProvider;

import android.util.Log;
import android.content.pm.ActivityInfo;
import java.io.File;
import java.util.Objects;

import org.akkord.lib.PlayServicesManager;
import org.akkord.lib.Utils;

import static android.app.PendingIntent.FLAG_IMMUTABLE;

public class CiTestActivity extends AkkordActivity {
    private PlayServicesManager mPlayServicesManager;

    private long last_XSmall  = 0;
    private long last_Small   = 0;
    private long last_Medium  = 0;
    private long last_Large   = 0;
    private long last_XLarge  = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(null);
        mPlayServicesManager = new PlayServicesManager(this, false);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(Utils.TAG, "onResume CiTestActivity()");

        // Since the state of the signed in user can change when the activity is not active
        // it is recommended to try and sign in silently from when the app resumes.
        signInSilently();
    }

    @Override
    public void setOrientationBis(int w, int h, boolean resizable, String hint) {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_USER);
    }

    private void signInSilently() {
        Log.d(Utils.TAG, "signInSilently()");
        mPlayServicesManager.signInSilently();
    }

    public void SignIn() {
        mPlayServicesManager.startSignInIntent();
    }

    public void SignOut() {
    }

    public void SubmitScore(long XSmall, long Small, long Medium, long Large, long XLarge) {
        try {
            if (mPlayServicesManager.isAuthenticated()) {
                if(last_XSmall != XSmall) {
                    mPlayServicesManager.submitScore("000000000000000000", XSmall);
                }

                if(last_Small != Small) {
                    mPlayServicesManager.submitScore("000000000000000000", Small);
                }

                if(last_Medium != Medium) {
                    mPlayServicesManager.submitScore("000000000000000000", Medium);
                }

                if(last_Large != Large) {
                    mPlayServicesManager.submitScore("000000000000000000", Large);
                }

                if(last_XLarge != XLarge) {
                    mPlayServicesManager.submitScore("000000000000000000", XLarge);
                }

                last_XSmall  = XSmall;
                last_Small   = Small ;
                last_Medium  = Medium;
                last_Large   = Large ;
                last_XLarge  = XLarge;
            }
        }
        catch(Exception e) {
            Log.e(Utils.TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public void ShowLeaderboards() {
        mPlayServicesManager.showLeaderboards();
    }



    private void handleException(Exception exception, String details) {
        Log.e(Utils.TAG, exception.getMessage() + "; Details: " + details);
    }

    public void SaveSnapshot(final byte[] Data) {
        mPlayServicesManager.saveSnapshot(Data);
    }

    public void LoadSnapshot() {
        mPlayServicesManager.loadSnapshot();
    }

    public void DeleteSnapshot() {

    }

    public void ShowSnapshots() {
    }

    private void ShareFile(final String Title, final String Fname, final String MimeType) {
        try {
            class OneShotTask implements Runnable {
                OneShotTask() {
                }

                public void run() {
                    try {
                        Log.v(Utils.TAG, "ShareFile started: " + Fname);
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
                        } else {
                            startActivity(Intent.createChooser(sendIntent, Title));
                        }
                    } catch (Exception e) {
                        Log.e(Utils.TAG, Objects.requireNonNull(e.getMessage()));
                    }
                }
            };
            runOnUiThread(new OneShotTask());
        }
        catch(Exception e) {
            Log.e(Utils.TAG, Objects.requireNonNull(e.getMessage()));
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
            Log.e(Utils.TAG, "Error while log event: " + e.getMessage());
        }
        */
    }
}
