package org.akkord.lib;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import java.io.File;
import java.util.Locale;
import android.widget.Toast;
import android.util.Log;
import android.app.*;
import android.content.*;
import android.content.res.AssetManager;
import android.media.AudioManager;
import android.content.pm.PackageInfo;

import com.google.android.gms.tasks.Task;
import com.google.android.play.core.review.ReviewManager;
import com.google.android.play.core.review.ReviewInfo;
import com.google.android.play.core.review.ReviewManagerFactory;

public class Utils {
    private static  final String TAG = "SDL";
    private static  Activity _context = null;
    private static  AssetManager AssetMgr = null;

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
            if (android.os.Build.VERSION.SDK_INT >= 17) { // getProperty requires api level 17
                AudioManager am = (AudioManager) _context.getSystemService(Context.AUDIO_SERVICE);
                sampleRate = Integer.parseInt(am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE));
            }
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }

        if (sampleRate == 0) {
            return 44100; // Use a default value if property not found
        }
        return sampleRate;
    }

    public static int getAudioOutputBufferSize() { // https://developer.android.com/ndk/guides/audio/audio-latency
        int framesPerBufferInt = 0;
        try {
            if (android.os.Build.VERSION.SDK_INT >= 17) { // getProperty requires api level 17
                AudioManager am = (AudioManager) _context.getSystemService(Context.AUDIO_SERVICE);
                framesPerBufferInt = Integer.parseInt(am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER));
            }
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
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

    public static void showMessageBox(int Code, String Title, String Message, String Button1, String Button2, String Button3, long TimeOut)
    {
        class OneShotTask implements Runnable {
            private int    msgCode;
            private String msgTitle;
            private String msgMessage;
            private String msgButton1;
            private String msgButton2;
            private String msgButton3;
            private long   msgTimeOut;

            private AlertDialog mAlertDialog;

            class TimeOutRunnable implements Runnable {
                public void run() {
                    try {
                        Thread.sleep(msgTimeOut);
                        if(mAlertDialog != null)
                            if(mAlertDialog.isShowing())
                                mAlertDialog.dismiss();
                    } catch (InterruptedException e) {
                        Log.e(TAG, "InterruptedException from a ping check thread!");
                    }
                }
            }
            private Thread mTimeOutRunnable = null;

            // https://stackoverflow.com/questions/5853167/runnable-with-a-parameter
            OneShotTask(int Code, String Title, String Message, String Button1, String Button2, String Button3, long TimeOut)
            {
                msgCode    = Code;
                msgTitle   = Title;
                msgMessage = Message;
                msgButton1 = Button1;
                msgButton2 = Button2;
                msgButton3 = Button3;
                msgTimeOut = TimeOut;
            }
            public void run() {
                try
                {
                    /* пример с картинкой
                    ImageView image = new ImageView(_context);
                    image.setImageResource(R.drawable.icon);

                    AlertDialog.Builder builder =
                            new AlertDialog.Builder(_context, android.R.style.Theme_Material_Dialog_Alert).
                            setMessage("Message above the image").
                            setView(image).
                            setPositiveButton("OK", new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                        dialog.dismiss();
                                }
                            });
                    builder.create().show();
                    * */

                    AlertDialog.Builder builder;
                    //android.os.Build.VERSION.SDK_INT
                    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
                        builder = new AlertDialog.Builder(_context, android.R.style.Theme_Material_Dialog_Alert);
                    } else {
                        builder = new AlertDialog.Builder(_context);
                    }
                    builder.setTitle(msgTitle)
                            .setMessage(msgMessage)
                            .setPositiveButton(msgButton1, new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    Log.v(TAG, "PositiveButton click");
                                    MessageBoxCallback(msgCode, 1);
                                }
                            });

                    //builder.setIcon(android.R.drawable.ic_dialog_alert);
                    builder.setIconAttribute(android.R.attr.alertDialogIcon);

                    if(msgButton2 != null && !msgButton2.isEmpty()) {
                        if (msgButton3 != null && !msgButton3.isEmpty()) {
                            builder.setNeutralButton(msgButton2, new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    Log.v(TAG, "NeutralButton click");
                                    MessageBoxCallback(msgCode, 2);
                                }
                            });

                            builder.setNegativeButton(msgButton3, new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    Log.v(TAG, "NegativeButton click");
                                    MessageBoxCallback(msgCode, 3);
                                }
                            });
                        } else {
                            builder.setNegativeButton(msgButton2, new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    Log.v(TAG, "NegativeButton click");
                                    MessageBoxCallback(msgCode, 2);
                                }
                            });
                        }
                    }

                    builder.setOnCancelListener(new DialogInterface.OnCancelListener() {
                        @Override
                        public void onCancel(DialogInterface dialog) {
                            Log.v(TAG, "setOnCancelListener: onCancel");
                            MessageBoxCallback(msgCode, 0);
                        }
                    });

                    /*
                    if (android.os.Build.VERSION.SDK_INT >= 17)
                        builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
                            @Override
                            public void onDismiss(DialogInterface dialog) {
                                Log.v(TAG, "setOnCancelListener: onDismiss");
                                MessageBoxCallback(msgCode, 550);
                            }
                        });
                    */
                    mAlertDialog = builder.create();
                    mAlertDialog.setIcon(0); // убираем иконку
                    mAlertDialog.show();
                    if(msgTimeOut > 0)
                    {
                        mTimeOutRunnable = new Thread(new TimeOutRunnable());
                        mTimeOutRunnable.start();
                    }
                }
                catch(Exception e)
                {
                    Log.e(TAG, e.getMessage());
                }
            }
        }
        Log.v(TAG, "before runOnUiThread");
        _context.runOnUiThread(new OneShotTask(Code, Title, Message, Button1, Button2, Button3, TimeOut));
        Log.v(TAG, "after runOnUiThread");
    }

    public static void shareText(String Title, String Message)
    {
        try {
            if (android.os.Build.VERSION.SDK_INT >= 14)
            {
                class OneShotTask implements Runnable {
                    String shareTitle;
                    String shareMessage;

                    OneShotTask(String pTitle, String pMessage)
                    {
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
                            Log.e(TAG, e.getMessage());
                        }
                    }
                };

                _context.runOnUiThread(new OneShotTask(Title, Message));
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
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

    public static void showToast(String Msg, int Duration, int Gravity, int xOffset, int yOffset){
        try
        {
            Log.v(TAG, "showToast started");
            class OneShotTask implements Runnable {
                String toastMsg;
                int toastDuration;
                int toastGravity;
                int toastxOffset;
                int toastyOffset;

                OneShotTask(String Msg, int Duration, int Gravity, int xOffset, int yOffset) {
                    toastMsg = Msg;
                    toastDuration = Duration;
                    toastGravity = Gravity;
                    toastxOffset = xOffset;
                    toastyOffset = yOffset;
                }

                public void run() {
                    try
                    {
                        Log.v(TAG, "showToast makeTest " +  Integer.toString(toastGravity));

                        Toast toast = Toast.makeText(_context, toastMsg, toastDuration);
                        if (toastGravity >= 0) {
                            Log.v(TAG, "showToast set toastGravity");
                            toast.setGravity(toastGravity, toastxOffset, toastyOffset);
                        }
                        toast.show();
                        Log.v(TAG, "showToast after show");
                    }
                    catch(Exception e)
                    {
                        Log.e(TAG, e.getMessage());
                    }
                }
            }
            _context.runOnUiThread(new OneShotTask(Msg, Duration, Gravity, xOffset, yOffset));
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
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
            Log.e(TAG, e.getMessage());
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
            Log.e(TAG, e.getMessage());
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
            Log.e(TAG, e.getMessage());
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
            Log.e(TAG, e.getMessage());
            return 1; // Error
        }
    }

    public static String GetAppVersionName() {
        try {
            final PackageInfo pInfo = _context.getPackageManager().getPackageInfo(_context.getPackageName(), 0);
            return pInfo.versionName;
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
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
            Log.e(TAG, e.getMessage());
            return "";
        }
    }

    public static void LaunchAppReviewIfAvailable() {
        try {
            Log.d(TAG, "LaunchAppReviewIfAvailable");
            ReviewManager reviewManager = ReviewManagerFactory.create(_context);
            if(null != reviewManager) {
                Log.d(TAG, "requestReviewFlow");
                Task<ReviewInfo> requestFlow = reviewManager.requestReviewFlow();
                requestFlow.addOnCompleteListener(task -> {
                    if (task.isSuccessful()) {
                        try {
                            // We can get the ReviewInfo object
                            Log.d(TAG, "requestReviewFlow isSuccessful");
                            ReviewInfo reviewInfo = task.getResult();
                            if(null != reviewInfo) {
                                Log.d(TAG, "launchReviewFlow");
                                Task<Void> flow = reviewManager.launchReviewFlow(_context, reviewInfo);
                                //flow.addOnCompleteListener(flowTask -> {
                                //    // The flow has finished. The API does not indicate whether the user
                                //    // reviewed or not, or even whether the review dialog was shown. Thus, no
                                //    // matter the result, we continue our app flow.
                                //    if (flowTask.isSuccessful()) {
                                //        Log.d(TAG, "launchReviewFlow isSuccessful");
                                //    }
                                //    else {
                                //        Log.d(TAG, "launchReviewFlow is not Successful");
                                //    }
                                //});
                            }
                        }
                        catch (Exception e) {
                            Log.e(TAG, e.getMessage());
                        }
                    } else {
                        Log.d(TAG, "requestReviewFlow is NOT Successful");
                        // There was some problem, continue regardless of the result.
                    }
                });
            }
        }
        catch (Exception e) {
            Log.e(TAG, e.getMessage());
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

    //public static native void runOnGameThread(Runnable task);
}