package org.akkord.lib;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import java.io.File;
import java.util.Locale;
import java.util.Objects;

import android.net.Uri;
import android.widget.Toast;
import android.util.Log;
import android.app.*;
import android.content.res.AssetManager;
import android.media.AudioManager;
import android.content.pm.PackageInfo;
import org.akkord.lib.AndroidStoreFacade;

public class Utils {
    public  static final String TAG = "SDL";
    private static Activity _context = null;
    private static AssetManager AssetMgr = null;

    public static native void MessageBoxCallback(int Code, int Result);

    public static void Init(Activity ActivityContext){
        _context = ActivityContext;
        AssetMgr = _context.getResources().getAssets();
    }

    public static Activity GetContext()
    {
        return _context;
    }

    public static int getAudioOutputRate() { // https://developer.android.com/ndk/guides/audio/audio-latency
        int sampleRate = 0;
        try {
            // getProperty requires api level 17
            AudioManager am = (AudioManager) _context.getSystemService(Context.AUDIO_SERVICE);
            sampleRate = Integer.parseInt(am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE));
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }

        if (sampleRate == 0) {
            return 44100; // Use a default value if property not found
        }
        return sampleRate;
    }

    public static int getAudioOutputBufferSize() { // https://developer.android.com/ndk/guides/audio/audio-latency
        int framesPerBufferInt = 0;
        try {
            // getProperty requires api level 17
            AudioManager am = (AudioManager) _context.getSystemService(Context.AUDIO_SERVICE);
            framesPerBufferInt = Integer.parseInt(am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER));
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }

        if (framesPerBufferInt == 0) {
            return 256; // Use a default
        }
        return framesPerBufferInt;
    }

    public static AssetManager GetAssetManager(){
        //return getResources().getAssets();
        //Log.v(TAG, "GetAssetManager()");
        return AssetMgr;
    }

    public static String getLanguage() {
        return Locale.getDefault().getLanguage();
    }

    public static String getPackage() {
        return _context.getPackageName();
    }

    public static void showMessageBox(final int msgCode, final String msgTitle, final String msgMessage, final String msgButton1, final String msgButton2, final String msgButton3, final long msgTimeOut) {
        try {
            _context.runOnUiThread(() ->
            {
                try {
                    AlertDialog.Builder builder;
                    //android.os.Build.VERSION.SDK_INT
                    builder = new AlertDialog.Builder(_context, android.R.style.Theme_Material_Dialog_Alert);
                    builder.setTitle(msgTitle)
                            .setMessage(msgMessage)
                            .setPositiveButton(msgButton1, (dialog, which) -> {
                                Log.v(TAG, "PositiveButton click");
                                MessageBoxCallback(msgCode, 1);
                            });

                    builder.setIconAttribute(android.R.attr.alertDialogIcon);

                    if (msgButton2 != null && !msgButton2.isEmpty()) {
                        if (msgButton3 != null && !msgButton3.isEmpty()) {
                            builder.setNeutralButton(msgButton2, (dialog, which) -> {
                                Log.v(TAG, "NeutralButton click");
                                MessageBoxCallback(msgCode, 2);
                            });

                            builder.setNegativeButton(msgButton3, (dialog, which) -> {
                                Log.v(TAG, "NegativeButton click");
                                MessageBoxCallback(msgCode, 3);
                            });
                        } else {
                            builder.setNegativeButton(msgButton2, (dialog, which) -> {
                                Log.v(TAG, "NegativeButton click");
                                MessageBoxCallback(msgCode, 2);
                            });
                        }
                    }

                    builder.setOnCancelListener(dialog -> {
                        Log.v(TAG, "setOnCancelListener: onCancel");
                        MessageBoxCallback(msgCode, 0);
                    });

                    final AlertDialog mAlertDialog = builder.create();
                    mAlertDialog.setIcon(0); // убираем иконку
                    mAlertDialog.show();

                    if (msgTimeOut > 0) {
                        Thread mTimeOutThread = new Thread(() -> {
                            try {
                                Thread.sleep(msgTimeOut);
                                if (mAlertDialog != null)
                                    if (mAlertDialog.isShowing()) {
                                        mAlertDialog.dismiss();
                                    }
                            } catch (Exception e) {
                                Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                            }
                        });
                        mTimeOutThread.start();
                    }
                }
                catch(Exception e) {
                    Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                }
            });
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public static void shareText(String Title, String Message)
    {
        try {
            class OneShotTask implements Runnable {
                private final String shareTitle;
                private final String shareMessage;

                OneShotTask(String pTitle, String pMessage) {
                    shareTitle = pTitle;
                    shareMessage = pMessage;
                }

                public void run() {
                    try {
                        Intent sendIntent = new Intent();
                        sendIntent.setAction(Intent.ACTION_SEND);
                        sendIntent.putExtra(Intent.EXTRA_TEXT, shareMessage);
                        sendIntent.setType("text/plain");
                        _context.startActivity(Intent.createChooser(sendIntent, shareTitle));
                    } catch (Exception e) {
                        Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                    }
                }
            }

            _context.runOnUiThread(new OneShotTask(Title, Message));
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    /*
    private static void ShareFile(final String Title, final String Fname, final String MimeType) {
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
                                            _context,
                                            _context.getPackageName() + ".provider",
                                            new File(Fname))
                                    );
                             _context.startActivity(Intent.createChooser(sendIntent, Title));
                        } catch (Exception e) {
                            Log.e(TAG, e.getMessage());
                        }
                    }
                };
                _context.runOnUiThread(new OneShotTask());
            }
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void sharePNG(final String Title, final String Fname) {
        ShareFile(Title, Fname, "image/png");
    }
    */

    public static void openURL(String url) {
        try {
            //_context.openURL(url);
            Intent browseIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
            _context.startActivity(browseIntent);
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public static void openGooglePlayURL(final String appPackageName) {
        try {
            _context.startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("https://play.google.com/store/apps/details?id=" + appPackageName)));
        }
        catch(Exception ex) {
            Log.e(TAG, Objects.requireNonNull(ex.getMessage()));
        }
    }

    public static void openCurrentAppGooglePlayURL() {
        try {
            openGooglePlayURL(_context.getPackageName());
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public static void showToast(final String Msg, final int Duration, final int Gravity, final int xOffset, final int yOffset){
        try {
            _context.runOnUiThread(() -> {
                try {
                    Toast toast = Toast.makeText(_context, Msg, Duration);
                    if (Gravity >= 0) {
                        toast.setGravity(Gravity, xOffset, yOffset);
                    }
                    toast.show();
                }
                catch(Exception e) {
                    Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public static int GetApiLevel()
    {
        return android.os.Build.VERSION.SDK_INT;
    }

    public static String GetInternalWriteDir()
    {
        return _context.getFilesDir().getAbsolutePath();
    }

    public static String GetInternalDir()
    {
        return _context.getApplicationInfo().dataDir;
    }

    public static int DirectoryExists(String Dir)
    {
        try
        {
            File file = new File(Dir);
            if(file.exists())
            {
                if(file.isDirectory()) return 2;
                return 3;
            }
            return 0;
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
            return 0;
        }
    }

    public static void deleteRecursive(File fileOrDirectory)
    {
        try
        {
            if (fileOrDirectory.isDirectory())
                for (File child : fileOrDirectory.listFiles())
                    deleteRecursive(child);

            fileOrDirectory.delete();
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public static int DirectoryDelete(String Dir, int Recursive)
    {
        try
        {
            File file = new File(Dir);

            if(file.exists())
            {
                if(file.isDirectory())
                {
                    if(Recursive == 1)
                    {
                        deleteRecursive(file);
                        return 0;
                    }
                    else
                    {
                        if(file.delete()) return 0;
                        else              return 2; // delete error
                    }
                }
                else
                {
                    // is's file
                    return 1;
                }
            }
            return 0;
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
            return 2; // delete error
        }
    }

    public static int MkDir(String path)
    {
        try
        {
            File file = new File(path);

            if(file.exists())
            {
                if(file.isDirectory())
                {
                    return 0;
                }
                else
                {
                    return 2; // It' a file and it's exists
                }
            }

            if(file.mkdirs())
            {
                return 0; // Success
            }
            else
            {
                return 1; // Error
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
            return 1; // Error
        }
    }

    public static String GetAppVersionName() {
        try {
            final PackageInfo pInfo = _context.getPackageManager().getPackageInfo(_context.getPackageName(), 0);
            return pInfo.versionName;
        } catch (Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
            return "";
        }
    }

    public static String GetAppVersionCode() {
        try {
            final PackageInfo pInfo = _context.getPackageManager().getPackageInfo(_context.getPackageName(), 0);
            if (android.os.Build.VERSION.SDK_INT < 28) {
                return Long.toString(pInfo.versionCode);
            }
            else {
                return Long.toString(pInfo.getLongVersionCode());
            }
        } catch (Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
            return "";
        }
    }

    public static void LaunchAppReviewIfAvailable() {
        try {
            AndroidStoreFacade.launchAppReviewIfPossible();
        }
        catch (Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public static void RequestFlexibleUpdateIfAvailable() {
        try {
             /*
            Log.d(TAG, "RequestFlexibleUpdateIfAvailable");
            // Creates instance of the manager.
            AppUpdateManager appUpdateManager = AppUpdateManagerFactory.create(_context);

            // Returns an intent object that you use to check for an update.
            Task<AppUpdateInfo> appUpdateInfoTask = appUpdateManager.getAppUpdateInfo();

            // Checks that the platform will allow the specified type of update.
            appUpdateInfoTask.addOnSuccessListener(appUpdateInfo -> {
                try {
                    Log.d(TAG, "add OnSuccessListener");
                    if (appUpdateInfo.updateAvailability() == UpdateAvailability.UPDATE_AVAILABLE
                        // For a flexible update, use AppUpdateType.FLEXIBLE
                        && appUpdateInfo.isUpdateTypeAllowed(AppUpdateType.FLEXIBLE)) {
                            // Request the update.

                        Log.d(TAG, "OnSuccessListener Request the update.");
                        InstallStateUpdatedListener listener = state -> {
                            // (Optional) Provide a download progress bar.
                            if (state.installStatus() == InstallStatus.DOWNLOADED) {
                                try {
                                    Log.d(TAG, "DOWNLOADED. Snackbar.make");
                                    Snackbar snackbar = Snackbar.make(
                                        _context.getWindow().getDecorView().getRootView(),
                                        "An update has just been downloaded.",
                                        Snackbar.LENGTH_LONG
                                    );
                                    Log.d(TAG, "setAction RESTART");
                                    snackbar.setAction("RESTART", view -> appUpdateManager.completeUpdate());
                                    //snackbar.setActionTextColor(getResources().getColor(R.color.snackbar_action_text_color));
                                    Log.d(TAG, "setAction Color");
                                    snackbar.show();
                                }
                                catch (Exception e) {
                                    Log.e(TAG, e.getMessage());
                                }
                            }
                            // Log state or install the update.
                        };

                        // Before starting an update, register a listener for updates.
                        appUpdateManager.registerListener(listener);

                        appUpdateManager.startUpdateFlowForResult(
                            // Pass the intent that is returned by 'getAppUpdateInfo()'.
                            appUpdateInfo,
                            // Or 'AppUpdateType.FLEXIBLE' for flexible updates.
                            AppUpdateType.FLEXIBLE,
                            // The current activity making the update request.
                            _context,
                            // Include a request code to later monitor this update request.
                            -100999);
                    }
                }
                catch (Exception e) {
                    Log.e(TAG, e.getMessage());
                }
            });
            */
        }
        catch (Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void checkUpdate() {
        try {
            AndroidStoreFacade.checkAppUpdate();
        } catch (Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public static void onActivityStop() {
        try {
            AndroidStoreFacade.onActivityStop();
        } catch (Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }
}