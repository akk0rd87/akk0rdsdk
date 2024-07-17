package org.popapp.citest;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import static android.content.Intent.EXTRA_CHOSEN_COMPONENT;
import static org.popapp.citest.MainActivity.TAG;

public class MyBroadcastReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        try {
            //ComponentName clickedComponent = intent.getParcelableExtra(EXTRA_CHOSEN_COMPONENT);
            //Log.v(TAG, "MyBroadcastReceiver: onReceive: " + clickedComponent.toShortString());
            //org.popapp.citest.MainActivity.logImageSendEvent(clickedComponent.toShortString());
        }
        catch(Exception e) {
            //Log.e(TAG, e.getMessage());
        }
    }
}