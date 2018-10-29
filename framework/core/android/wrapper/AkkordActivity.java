package org.akkord.lib;

import android.content.Intent;
import android.os.Bundle;
import org.libsdl.app.SDLActivity;
import android.content.pm.ActivityInfo;
import android.app.Activity;


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
    
    protected void onActivityResult(int requestCode, int resultCode, Intent data) 
    {
        super.onActivityResult(requestCode, resultCode, data);
        nativeOnActivityResult(this, requestCode, resultCode, data);
    }
    
    
    
    
    // NATIVE
    private static native void nativeOnActivityResult(Activity activity, int requestCode, int resultCode, Intent data);
}
