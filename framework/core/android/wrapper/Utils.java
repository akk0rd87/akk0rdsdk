package org.akkord.lib;

import android.app.Activity;
import android.app.AlarmManager;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.provider.Settings;
import android.app.PendingIntent;
import android.os.StatFs;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Locale;
import android.widget.Toast;
import android.util.Log;

import android.widget.TextView;
import android.content.pm.ActivityInfo;
import android.app.*;
import android.content.*;
import android.text.InputType;
import android.content.res.AssetManager;
//import android.support.v4.app.DialogFragment;

public class Utils {
    private static  final String TAG = "SDL";
    private static  Activity _context;
    private static  AssetManager AssetMgr;
    //private static Context context;
    
    private static String ToastMessage;
    private static int    ToastLength;
    private static int    ToastGravity;
    private static int    ToastxOffset;
    private static int    ToastyOffset;
    
    public static native void AkkordCallback(String str);

    public static void Init(Activity ActivityContext){
        _context = ActivityContext;
        AssetMgr = _context.getResources().getAssets();
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

    public static void openURL(String url) {
        Log.v(TAG, "openURL 1");
        //AkkordCallback("Hello world");
        Log.v(TAG, "openURL 2");
        //Intent browseIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
        //_context.startActivity(browseIntent);
        _context.runOnUiThread(new Runnable() {
            public void run() {              
                 AlertDialog.Builder builder;
                 //android.os.Build.VERSION.SDK_INT
                    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
                        builder = new AlertDialog.Builder(_context, android.R.style.Theme_Material_Dialog_Alert);
                    } else {
                        builder = new AlertDialog.Builder(_context);
                    }
                    builder.setTitle("Delete entry")
                    .setMessage("Are you sure you want to delete this entry?")
                    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) { 
                            // continue with delete
                        }
                     })
                    .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) { 
                            // do nothing
                        }
                     })
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .show();
            }
        });             
        
        /*
        Log.v(TAG, "00001");
        AlertDialog alertDialog = new AlertDialog.Builder(_context).create();
        Log.v(TAG, "00002");
        alertDialog.setTitle("Alert");
        Log.v(TAG, "00003");
        alertDialog.setMessage("Alert message to be shown");
        Log.v(TAG, "00004");
        alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",        
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    dialog.dismiss();
                }
            });
        Log.v(TAG, "00005");
        alertDialog.show(); 
        Log.v(TAG, "00006");
        */
    }
    
    public static void showToast(String Msg, int Duration, int Gravity, int xOffset, int yOffset){
        //Log.v(TAG, "showToast() before: " + Msg + " Duration " + Duration);
        
        ToastMessage = Msg;
        ToastLength  = Duration;
        ToastGravity = Gravity;
        ToastxOffset = xOffset;
        ToastyOffset = yOffset;        

        _context.runOnUiThread(new Runnable() {
            public void run() {                
                if(ToastLength >= 0)
                {
                    Toast toast = Toast.makeText(_context, ToastMessage, ToastLength);
                    if(ToastGravity >= 0) toast.setGravity(ToastGravity, ToastxOffset, ToastyOffset);                    
                    toast.show();
                }
                
                ToastLength  = -1;
                ToastMessage = null;
            }
        });        
        
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
        File file = new File(Dir);        
        if(file.exists()) 
        {
            if(file.isDirectory()) return 2;
            return 3;
        }
        return 0;
    }
    
    public static void deleteRecursive(File fileOrDirectory) 
    {
        if (fileOrDirectory.isDirectory())
            for (File child : fileOrDirectory.listFiles())
                deleteRecursive(child);

        fileOrDirectory.delete();
    }    
    
    public static int DirectoryDelete(String Dir, int Recursive)
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
    
    public static int MkDir(String path)
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
    
    public static native void runOnGameThread(Runnable task);
}