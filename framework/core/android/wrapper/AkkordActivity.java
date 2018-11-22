package org.akkord.lib;

import org.libsdl.app.SDLActivity;
import android.os.Bundle;
import android.app.Activity;

public class AkkordActivity extends SDLActivity {
    
    //protected static AkkordActivity instance;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);        
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
}
