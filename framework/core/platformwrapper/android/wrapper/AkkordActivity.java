package org.akkord.lib;

import org.libsdl.app.SDLActivity;
import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import com.getkeepsafe.relinker.ReLinker;

import com.google.android.play.core.appupdate.AppUpdateInfo;
import com.google.android.play.core.appupdate.AppUpdateManager;
import com.google.android.play.core.appupdate.AppUpdateManagerFactory;
import com.google.android.play.core.install.InstallState;
import com.google.android.play.core.install.InstallStateUpdatedListener;
import com.google.android.play.core.install.model.AppUpdateType;
import com.google.android.play.core.install.model.InstallStatus;
import com.google.android.play.core.install.model.UpdateAvailability;
import com.google.android.play.core.tasks.OnSuccessListener;
import com.google.android.play.core.tasks.Task;

public class AkkordActivity extends SDLActivity {
    private static String TAG = "SDL";
    private AppUpdateManager mAppUpdateManager = null;
    private InstallStateUpdatedListener installStateUpdatedListener = null;

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
                Log.e("SDL", "Relinker ERROR!!! " + e.getMessage());
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
        checkUpdate();
    }

    @Override
    protected void onStop() {
        super.onStop();

        try {
            if (mAppUpdateManager != null && installStateUpdatedListener != null) {
                mAppUpdateManager.unregisterListener(installStateUpdatedListener);
            }
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    private void checkUpdate() {
        try {
            mAppUpdateManager = AppUpdateManagerFactory.create(this);

            installStateUpdatedListener = new
            InstallStateUpdatedListener() {
                @Override
                public void onStateUpdate(InstallState state) {
                    if (state.installStatus() == InstallStatus.DOWNLOADED){
                        //CHECK THIS if AppUpdateType.FLEXIBLE, otherwise you can skip
                        popupSnackbarForCompleteUpdate();
                    } else if (state.installStatus() == InstallStatus.INSTALLED){
                        if (mAppUpdateManager != null && installStateUpdatedListener != null){
                        mAppUpdateManager.unregisterListener(installStateUpdatedListener);
                        }
                    } else {
                        Log.i(TAG, "InstallStateUpdatedListener: state: " + state.installStatus());
                    }
                }
            };

            mAppUpdateManager.registerListener(installStateUpdatedListener);
            mAppUpdateManager.getAppUpdateInfo().addOnSuccessListener(appUpdateInfo -> {
                if (appUpdateInfo.updateAvailability() == UpdateAvailability.UPDATE_AVAILABLE) {
                        //&& appUpdateInfo.isUpdateTypeAllowed(AppUpdateType.FLEXIBLE /*AppUpdateType.IMMEDIATE*/))
                    try {
                        mAppUpdateManager.startUpdateFlowForResult(appUpdateInfo, AppUpdateType.FLEXIBLE /*AppUpdateType.IMMEDIATE*/, this, /*RC_APP_UPDATE*/ 100500);

                    } catch (Exception e) {
                        e.printStackTrace();
                        Log.e(TAG, e.getMessage());
                    }

                } else if (appUpdateInfo.installStatus() == InstallStatus.DOWNLOADED) {
                    //CHECK THIS if AppUpdateType.FLEXIBLE, otherwise you can skip
                    popupSnackbarForCompleteUpdate();
                } else {
                    Log.e(TAG, "checkForAppUpdateAvailability: something else");
                }
            });
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    private void popupSnackbarForCompleteUpdate() {
        try {
            if(mAppUpdateManager != null) {
                mAppUpdateManager.completeUpdate();
                Log.d(TAG, "popupSnackbarForCompleteUpdate");
            }
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }
}
