package org.akkord.lib;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.provider.Settings;
import android.os.StatFs;
import java.io.File;
import java.util.Locale;
import android.widget.Toast;
import android.util.Log;
import android.app.*;
import android.content.*;
import android.content.res.AssetManager;
import android.media.AudioManager;

import com.google.android.play.core.tasks.Task;
import com.google.android.play.core.review.ReviewManager;
import com.google.android.play.core.review.ReviewInfo;
import com.google.android.play.core.review.ReviewManagerFactory;
import com.google.android.play.core.review.ReviewInfo;

public class Utils {
    private static  final String TAG = "SDL";
    private static  AkkordActivity _context = null;
    private static  AssetManager AssetMgr = null;

    private static  ReviewManager reviewManager = null;
    private static  ReviewInfo    reviewInfo = null;

    public static native void MessageBoxCallback(int Code, int Result);

    public static void Init(AkkordActivity ActivityContext){
        _context = ActivityContext;
        AssetMgr = _context.getResources().getAssets();
        InitReviewSettings();
    }

    public static Activity GetContext()
    {
        return _context;
    }

    public static long getTimeUTCMS() {
        return System.currentTimeMillis();
    }

    public static long getFreeSpace(String path) {
        StatFs fs = new StatFs(path);
        long blocks = fs.getFreeBlocks();
        long blsize = fs.getBlockSize();
        return blocks * blsize;
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

    public static String getProperty(String prop) {
        if (prop == "ANDROID_ID")
            return Settings.Secure.ANDROID_ID;
        return "";
    }

    public static boolean isNetworkAvailable() {
        if (_context == null)
            return false;
        ConnectivityManager connectivityManager = (ConnectivityManager) _context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
        return activeNetworkInfo != null;
    }

    public static void exit() {
        System.exit(0);
    }

    public static void moveTaskToBack() {
        _context.moveTaskToBack(true);
    }

//    public static void restartApp() {
//        Intent mStartActivity = new Intent(_context, OxygineActivity.instance.getClass());
//        int mPendingIntentId = 123456;
//        PendingIntent mPendingIntent = PendingIntent.getActivity(_context, mPendingIntentId, mStartActivity, PendingIntent.FLAG_CANCEL_CURRENT);
//        AlarmManager mgr = (AlarmManager)_context.getSystemService(Context.ALARM_SERVICE);
//        mgr.set(AlarmManager.RTC, System.currentTimeMillis() + 100, mPendingIntent);
//        System.exit(0);
//    }

    //public static void openURL(String url) {
    //    try
    //    {
    //        _context.openURL(url);
    //        //Intent browseIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
    //        //_context.startActivity(browseIntent);
    //    }
    //    catch(Exception e)
    //    {
    //        Log.e(TAG, e.getMessage());
    //    }
    //}

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

    private static void InitReviewSettings() {
        try {
            reviewManager = ReviewManagerFactory.create(_context);
            Task<ReviewInfo> requestFlow = reviewManager.requestReviewFlow();
            requestFlow.addOnCompleteListener(task -> {
                if (task.isSuccessful()) {
                    // We can get the ReviewInfo object
                    reviewInfo = task.getResult();
                } else {
                    // There was some problem, continue regardless of the result.
                }
            });
        }
        catch (Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void LaunchAppReviewIfAvailable() {
        try {
            if(null != reviewManager && null != reviewInfo) {
                Log.d(TAG, "launchReviewFlow");
                Task<Void> flow = reviewManager.launchReviewFlow(_context, reviewInfo);
                flow.addOnCompleteListener(task -> {
                    Log.d(TAG, "launchReviewFlow comleted");
                    // The flow has finished. The API does not indicate whether the user
                    // reviewed or not, or even whether the review dialog was shown. Thus, no
                    // matter the result, we continue our app flow.
                });
            }
        }
        catch (Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    public static native void runOnGameThread(Runnable task);
}