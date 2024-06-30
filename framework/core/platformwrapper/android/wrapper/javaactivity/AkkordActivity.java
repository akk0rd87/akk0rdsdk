package org.akkord.lib;

import org.libsdl.app.SDLActivity;
import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import com.getkeepsafe.relinker.ReLinker;

import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.gms.tasks.Task;

public class AkkordActivity extends SDLActivity {
    private static String TAG = "SDL";

    public void akkord_loadLibrary(String libraryName) throws UnsatisfiedLinkError, SecurityException, NullPointerException {

        if (libraryName == null) {
            throw new NullPointerException("No library name provided.");
        }

        try {
            ReLinker.loadLibrary(this, libraryName);
            Log.d("SDL", "Relinker library load success: " + libraryName);
        }
        catch (final Throwable e) {
            // Fall back
            try {
                Log.e("SDL", "Relinker load error :" + libraryName + ". " + e.getMessage());
                System.loadLibrary(libraryName);
            }
            catch (final UnsatisfiedLinkError ule) {
                throw ule;
            }
            catch (final SecurityException se) {
                throw se;
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        org.akkord.lib.Utils.Init(this);
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "c++_shared",
            "SDL2",
            //"SDL2_png",
            //"SDL2_jpeg",
             "SDL2_image",
            // "SDL2_mixer",
            // "SDL2_net",
            // "SDL2_ttf",
            "main"
        };
    }

    @Override
    public void loadLibraries() {
       for (String lib : getLibraries()) {
          //System.loadLibrary(lib);
          akkord_loadLibrary(lib);
       }
    }

    @Override
    protected void onStart() {
        super.onStart();
        org.akkord.lib.Utils.checkUpdate();
    }

    @Override
    protected void onStop() {
        super.onStop();
        Utils.onActivityStop();
    }
}
