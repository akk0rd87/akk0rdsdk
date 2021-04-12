package org.akkord.lib;

import android.app.Activity;
import android.content.Intent;

//import com.google.playservices;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import android.util.Log;
import java.util.Map;
import com.google.android.gms.ads.initialization.AdapterStatus;
import com.google.android.gms.ads.initialization.AdapterStatus.State;
import com.google.android.gms.ads.initialization.InitializationStatus;
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;
import com.google.android.gms.ads.interstitial.InterstitialAd;
import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback;
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardItem;
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;
import com.google.android.gms.ads.OnUserEarnedRewardListener;
import com.google.android.gms.ads.AdError;
import com.google.android.gms.ads.LoadAdError;
import com.google.android.gms.ads.FullScreenContentCallback;
import androidx.annotation.NonNull;

public class AdMobAdapter extends AdListener implements OnInitializationCompleteListener {
    private static AdMobAdapter                  adMobAdapter                  = new AdMobAdapter();
    private static MyInterstitialCallback        mInterstitialCallbackListener = null;
    private static MyInterstitialContentCallback mInterstitialContentCallback  = null;
    private static MyRewardedCallback            mRewardedCallback             = null;
    private static MyRewardedContentCallback     mRewardedContentCallback      = null;

    public static void InterstitialInit() {
        try {
            mInterstitialCallbackListener = new MyInterstitialCallback();
            mInterstitialContentCallback  = new MyInterstitialContentCallback();
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    };

    public static void RewardedVideoInit() {
        try {
            mRewardedCallback        = new MyRewardedCallback();
            mRewardedContentCallback = new MyRewardedContentCallback();
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    };

    private static class MyInterstitialCallback extends InterstitialAdLoadCallback {
        @Override
        public void onAdLoaded(@NonNull InterstitialAd interstitialAd) {
            // The mInterstitialAd reference will be null until
            // an ad is loaded.
            if(null != interstitialAd) {
                mInterstitialAd = interstitialAd;
                AdCallback_Local(AD_INTERSTITIAL, EVENT_INTERSTITIAL_LOADED, 0);
                //Log.i(TAG, "onAdLoaded");
            }
            else {
                mInterstitialAd = null;
                AdCallback_Local(AD_INTERSTITIAL, EVENT_INTERSTITIAL_FAILED, 0);
            }
        }

        @Override
        public void onAdFailedToLoad(@NonNull LoadAdError loadAdError) {
            // Handle the error
            //Log.d(TAG, loadAdError.getMessage());
            mInterstitialAd = null;
            AdCallback_Local(AD_INTERSTITIAL, EVENT_INTERSTITIAL_FAILED, 0);
        }
    };

    private static class MyInterstitialContentCallback extends FullScreenContentCallback {
        @Override
        public void onAdDismissedFullScreenContent() {
          // Called when fullscreen content is dismissed.
          mInterstitialAd = null;
          Log.d("TAG", "The ad was dismissed.");
          AdCallback_Local(AD_INTERSTITIAL, EVENT_INTERSTITIAL_CLOSED, 0);
        }

        @Override
        public void onAdFailedToShowFullScreenContent(AdError adError) {
          // Called when fullscreen content failed to show.
          mInterstitialAd = null;
          Log.d("TAG", "The ad failed to show.");
          AdCallback_Local(AD_INTERSTITIAL, EVENT_INTERSTITIAL_FAILED, 0);
        }

        @Override
        public void onAdShowedFullScreenContent() {
          // Called when fullscreen content is shown.
          // Make sure to set your reference to null so you don't
          // show it a second time.
          mInterstitialAd = null;
          // тут вызываем Closed, так как обнулили ссылку и можно запращивать новую рекламу
          AdCallback_Local(AD_INTERSTITIAL, EVENT_INTERSTITIAL_CLOSED, 0);
          Log.d("TAG", "The ad was shown.");
        }
    };
    private static class MyRewardedCallback extends RewardedAdLoadCallback implements OnUserEarnedRewardListener {
        @Override
        public void onAdFailedToLoad(@NonNull LoadAdError loadAdError) {
          // Handle the error.
          Log.d(TAG, loadAdError.getMessage());
          mRewardedAd = null;
          AdCallback_Local(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_FAILED, 0);
        }

        @Override
        public void onAdLoaded(@NonNull RewardedAd rewardedAd) {
            if(null != rewardedAd) {
                mRewardedAd = rewardedAd;
                //Log.d(TAG, "onAdFailedToLoad");
                AdCallback_Local(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_LOADED, 0);
            }
            else {
                mRewardedAd = null;
                AdCallback_Local(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_FAILED, 0);
            }
        }

        //
        // OnUserEarnedRewardListener Callback
        //
        @Override
        public void onUserEarnedReward(@NonNull RewardItem rewardItem) {
          // Handle the reward.
           Log.d("TAG", "The user earned the reward.");
           //int rewardAmount = rewardItem.getAmount();
           //String rewardType = rewardItem.getType();
           AdCallback_Local(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_REWARDED, 0);
           mRewardedAd = null;
        }
    };

    private static class MyRewardedContentCallback extends FullScreenContentCallback {
        @Override
        public void onAdShowedFullScreenContent() {
          // Called when ad is shown.
          Log.d(TAG, "Ad was shown.");
          mRewardedAd = null;
          AdCallback_Local(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_STARTED, 0);
        }

        @Override
        public void onAdFailedToShowFullScreenContent(AdError adError) {
          // Called when ad fails to show.
          Log.d(TAG, "Ad failed to show.");
          mRewardedAd = null;
          AdCallback_Local(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_FAILED, 0);
        }

        @Override
        public void onAdDismissedFullScreenContent() {
          // Called when ad is dismissed.
          // Don't forget to set the ad reference to null so you
          // don't show the ad a second time.
          mRewardedAd = null;
          Log.d(TAG, "Ad was dismissed.");
          AdCallback_Local(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_CLOSED, 0);
        };
    };

    private static String TAG = "SDL";

    private static final int EVENT_INTERSTITIAL_LOADED          = 1;
    private static final int EVENT_INTERSTITIAL_OPENED          = 2;
    private static final int EVENT_INTERSTITIAL_CLOSED          = 3;
    private static final int EVENT_INTERSTITIAL_FAILED          = 4;
    private static final int EVENT_INTERSTITIAL_LEFTAPPLICATION = 5;

    private static final int EVENT_REWARDEDVIDEO_LOADED          = 101;
    private static final int EVENT_REWARDEDVIDEO_OPENED          = 102;
    private static final int EVENT_REWARDEDVIDEO_CLOSED          = 103;
    private static final int EVENT_REWARDEDVIDEO_FAILED          = 104;
    private static final int EVENT_REWARDEDVIDEO_LEFTAPPLICATION = 105;
    private static final int EVENT_REWARDEDVIDEO_STARTED         = 106;
    private static final int EVENT_REWARDEDVIDEO_COMPLETED       = 107;
    private static final int EVENT_REWARDEDVIDEO_REWARDED        = 108;

    private static final int AD_INTERSTITIAL       = 1;
    private static final int AD_REWARDEDVIDEO      = 2;
    private static final int AD_NATIVEADSADVANCED  = 4;

    private static InterstitialAd mInterstitialAd = null;
    private static RewardedAd     mRewardedAd     = null;

    private static String InterstitialUnitID;
    private static String RewardedVideoUnitID;

    private static boolean mAdMobInitializationCompleted = false;

    private static native void AdCallback(int AdType, int EventType, int Code);

    private static void AdCallback_Local(int AdType, int EventType, int Code) {
        try {
            AdCallback(AdType, EventType, Code);
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    };

    public static void Initialize()
    {
        try {
            mAdMobInitializationCompleted = false;
            MobileAds.initialize(Utils.GetContext(), adMobAdapter);
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void InterstitialSetUnitId(final String ID)
    {
        try
        {
            //Log.v(TAG, "InterstitialSetUnitId start");
            InterstitialUnitID = ID;
            //Log.v(TAG, "InterstitialSetUnitId finish");
        }
        catch(Exception e)
        {
            Log.v(TAG, e.getMessage());
        }
    }

    public static String InterstitialGetUnitId()
    {
        try {
            //Log.v(TAG, "InterstitialGetAdUnitId start");
            return (null != InterstitialUnitID ? InterstitialUnitID : "");
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
            return null;
        }
    }

    public static void RewardedVideoSetUnitId(String ID)
    {
        try {
            RewardedVideoUnitID = ID;
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }

    public static String RewardedVideoGetUnitId()
    {
        try{
            return (null != RewardedVideoUnitID ? RewardedVideoUnitID : "");
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
            return null;
        }
    }

    public static void InterstitialLoad()
    {
        try
        {
            //Log.d(TAG,"InterstitialLoad");
            mInterstitialAd = null;

            if(null == mInterstitialCallbackListener) {
                Log.e(TAG, "Interstitial was not initialized");
                return;
            }
            if(mAdMobInitializationCompleted) {
                Utils.GetContext().runOnUiThread(new Runnable() {
                    public void run() {
                        try {
                            InterstitialAd.load(Utils.GetContext(), InterstitialUnitID, new AdRequest.Builder().build(), mInterstitialCallbackListener);
                        }
                        catch(Exception e) {
                            Log.e(TAG, e.getMessage());
                        }
                    }
                });
            }
            else {
                // если SDK не проинициализировано, имитурем, как будто загрузка прошла неуспешно
                AdCallback_Local(AD_INTERSTITIAL, EVENT_INTERSTITIAL_FAILED, 0);
            }
        }
        catch(Exception e)
        {
            Log.v(TAG, e.getMessage());
        }
    }

    public static void RewardedVideoLoad()
    {
        try
        {
            //Log.d(TAG,"RewardedVideoLoad");
            mRewardedAd = null;

            if(null == mRewardedCallback) {
                Log.e(TAG, "Rewarded was not initialized");
                return;
            }

            if(mAdMobInitializationCompleted) {
                Utils.GetContext().runOnUiThread(new Runnable() {
                    public void run() {
                        try {
                            RewardedAd.load(Utils.GetContext(), RewardedVideoUnitID, new AdRequest.Builder().build(), mRewardedCallback);
                        }
                        catch(Exception e) {
                            Log.e(TAG, e.getMessage());
                        }
                    }
                });
            }
            else {
                // если SDK не проинициализировано, имитурем, как будто загрузка прошла неуспешно
                AdCallback_Local(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_FAILED, 0);
            }
        }
        catch(Exception e)
        {
            Log.v(TAG, e.getMessage());
        }
    }

    public static int InterstitialShow()
    {
        try
        {
            Activity ctx = Utils.GetContext();
            ctx.runOnUiThread(new Runnable() {
                public void run() {
                    try {
                        if (null != mInterstitialAd) {
                            mInterstitialAd.setFullScreenContentCallback(mInterstitialContentCallback);
                            mInterstitialAd.show(Utils.GetContext());
                        }
                    }
                    catch(Exception e) {
                        Log.e(TAG, e.getMessage());
                    }
                }
            });

            return 0;
        }
        catch(Exception e)
        {
            Log.v(TAG, e.getMessage());
            return -1;
        }
    }

    public static int RewardedVideoShow()
    {
        try
        {
            Activity ctx = Utils.GetContext();
            ctx.runOnUiThread(new Runnable() {
                public void run() {
                    try {
                        if (null != mRewardedAd ) {
                            mRewardedAd.setFullScreenContentCallback(mRewardedContentCallback);
                            mRewardedAd.show(Utils.GetContext(), mRewardedCallback);
                        }
                    }
                    catch(Exception e) {
                        Log.e(TAG, e.getMessage());
                    }
                }
            });

            return 0;
        }
        catch(Exception e)
        {
            System.err.println(e.getMessage());
            Log.v(TAG, e.getMessage());
            return -1;
        }
    }

    ///////////////////////////////
    // CALLBACKS FOR OnInitializationCompleteListener
    ///////////////////////////////
    @Override
    public void onInitializationComplete(InitializationStatus initializationStatus) {
        try {
            //Log.v(TAG, "InitializationComplete,  status=" + initializationStatus);
            //Log.v(TAG, "InitializationComplete,  status=" + initializationStatus.getAdapterStatusMap());
            final Map<String, AdapterStatus> mp = initializationStatus.getAdapterStatusMap();

            if(null != mp) {
                for (Map.Entry<String, AdapterStatus> entry : mp.entrySet()) {
                    //Log.v(TAG, "Print entry before");
                    //Log.v(TAG, "Key = " + entry.getKey() + ", Value = " + entry.getValue());

                    if(AdapterStatus.State.READY == entry.getValue().getInitializationState()) {
                        mAdMobInitializationCompleted = true;
                        return;
                    }
                }
            }
        }
        catch(Exception e) {
            System.err.println(e.getMessage());
            Log.v(TAG, e.getMessage());
        }
    }
}