package org.akkord.lib;

import android.content.Intent;
import android.os.Bundle;
import org.libsdl.app.SDLActivity;
import android.content.pm.ActivityInfo;

public class AkkordActivity extends SDLActivity {
    
    //protected static AkkordActivity instance;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //instance = this;
        //Utils._context = this;
        Utils.Init(this);
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
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
    
    /*
    @Override
    public static void setOrientation(int w, int h, boolean resizable, String hint){
        mSingleton.setRequestedOrientation(-1);
    }
    */
    
    /*
    @Override
    public void setOrientationBis(int w, int h, boolean resizable, String hint) {
        // https://developer.android.com/guide/topics/manifest/activity-element.html#screen
        // https://developer.android.com/reference/android/content/pm/ActivityInfo.html#SCREEN_ORIENTATION_FULL_SENSOR
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR);
    } 
    */    
}
