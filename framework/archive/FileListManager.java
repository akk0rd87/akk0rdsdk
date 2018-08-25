package org.akkord.lib;

import java.io.File;
import android.util.Log;

public class FileListManager {
    private static String TAG = "SDL";
    private static File files[];
    private static File dir;
    
    public static void CloseList()
    {
        // To do clear manually ()
        files = null;
        dir   = null;
        //pointer = 0;
    }
    
    public static int GetList(String Path)
    {
        try
        {
            //Log.v(TAG, "Files: 1");
            CloseList();
            //Log.v(TAG, "Files: 2");
            dir   = new File(Path);
            //Log.v(TAG, "Files: 3: " + dir);
            
            files = dir.listFiles();
            
            //Log.v(TAG, "Files: " + files.length);

            return files.length;
        }
        
        catch(Exception e)
        {
            Log.v(TAG, "Exception: " + e);
            return 0;
        }
    }
    
    public static void GetNext(int Pointer)
    {
        Log.v(TAG, "Files: FileName:" + files[Pointer].getName() + " isDir:" + files[Pointer].isDirectory());
    }
    
    public static String GetFileName(int Pointer)
    {
        return files[Pointer].getName();
    }
    
     public static int GetIsDir(int Pointer)
    {
        if (files[Pointer].isDirectory()) return 1;
        return 0;
    }
}