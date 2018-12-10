package org.akkord.lib;

import android.app.Activity;
import android.content.Intent;

//import com.google.playservices;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.reward.RewardItem;
import com.google.android.gms.ads.reward.RewardedVideoAd;
import com.google.android.gms.ads.reward.RewardedVideoAdListener;
import android.util.Log;

public class AdMobAdapter {
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
    
    private static InterstitialAd mInterstitialAd;
    private static RewardedVideoAd mRewardedVideoAd;
    
    private static String RewardedVideoUnitID;
    
    public static native void AdCallback(int AdType, int EventType, int Code);
    
    public static void Initialize(String AdMobAppID)
    {
        MobileAds.initialize(Utils.GetContext(), AdMobAppID);
    }
    
    private static void InterstitialReCreate()
    {
        try {
            mInterstitialAd = new InterstitialAd(Utils.GetContext());
            mInterstitialAd.setAdListener(new AdListener() {
                @Override
                public void onAdLoaded() {
                    AdCallback(AD_INTERSTITIAL, EVENT_INTERSTITIAL_LOADED, 0);
                }

                @Override
                public void onAdFailedToLoad(int errorCode) {
                    String error;
                    switch (errorCode) {
                        case AdRequest.ERROR_CODE_INTERNAL_ERROR:
                            error = "ERROR_CODE_INTERNAL_ERROR";
                            break;
                        case AdRequest.ERROR_CODE_INVALID_REQUEST:
                            error = "ERROR_CODE_INVALID_REQUEST";
                            break;
                        case AdRequest.ERROR_CODE_NETWORK_ERROR:
                            error = "ERROR_CODE_NETWORK_ERROR";
                            break;
                        case AdRequest.ERROR_CODE_NO_FILL:
                            error = "ERROR_CODE_NO_FILL";
                            break;
                        default:
                            error = "Unknown";
                    }
                    //Log.v(TAG, "Interstitial FailedToLoad!!! [" + errorCode + ": " + error);
                    AdCallback(AD_INTERSTITIAL, EVENT_INTERSTITIAL_FAILED, errorCode);
                }

                @Override
                public void onAdOpened() {
                    //Log.v(TAG, "Interstitial Opened!!!");
                    AdCallback(AD_INTERSTITIAL, EVENT_INTERSTITIAL_OPENED, 0);
                }

                @Override
                public void onAdLeftApplication() {
                    //Log.v(TAG, "Interstitial onAdLeftApplication!!!");
                    AdCallback(AD_INTERSTITIAL, 4, EVENT_INTERSTITIAL_LEFTAPPLICATION);
                }

                @Override
                public void onAdClosed() {
                    //Log.v(TAG, "Interstitial Closed!!!");
                    AdCallback(AD_INTERSTITIAL, EVENT_INTERSTITIAL_CLOSED, 0);
                }
            });

        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void InterstitialInit()
    {

    }

    public static void RewardedVideoInit()
    {   
        try {
            Activity ctx = Utils.GetContext();
            ctx.runOnUiThread(new Runnable() {
                public void run() {

                    mRewardedVideoAd = MobileAds.getRewardedVideoAdInstance(Utils.GetContext());
                    mRewardedVideoAd.setRewardedVideoAdListener(new RewardedVideoAdListener() {
                        @Override
                        public void onRewardedVideoAdLoaded() {
                            AdCallback(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_LOADED, 0);
                        }

                        @Override
                        public void onRewardedVideoAdOpened() {
                            AdCallback(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_OPENED, 0);
                        }

                        @Override
                        public void onRewardedVideoStarted() {
                            AdCallback(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_STARTED, 0);
                        }

                        @Override
                        public void onRewardedVideoAdClosed() {
                            AdCallback(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_CLOSED, 0);
                        }

                        @Override
                        public void onRewarded(RewardItem rewardItem) {
                            AdCallback(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_REWARDED, 0);
                        }

                        @Override
                        public void onRewardedVideoAdLeftApplication() {
                            AdCallback(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_LEFTAPPLICATION, 0);
                        }

                        @Override
                        public void onRewardedVideoAdFailedToLoad(int errorCode) {
                            AdCallback(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_FAILED, errorCode);
                        }

                        @Override
                        public void onRewardedVideoCompleted() {
                            AdCallback(AD_REWARDEDVIDEO, EVENT_REWARDEDVIDEO_COMPLETED, 0);
                        }
                    });
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }
    
    public static void InterstitialSetUnitId(String ID)
    {
        try
		{
		    //Log.v(TAG, "InterstitialSetUnitId start");
            InterstitialReCreate();
            mInterstitialAd.setAdUnitId(ID);
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
            String id = mInterstitialAd.getAdUnitId();
            //Log.v(TAG, "InterstitialGetAdUnitId finish");
            return id;
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
            return RewardedVideoUnitID;
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
            Activity ctx = Utils.GetContext();
            ctx.runOnUiThread(new Runnable() {
                public void run() {
                    try{
                          mInterstitialAd.loadAd(new AdRequest.Builder().build());
                        }
                    catch(Exception e)
                    {

                    }
                }
            });
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
            Activity ctx = Utils.GetContext();
            ctx.runOnUiThread(new Runnable() {
                public void run() {
                    try {
                        mRewardedVideoAd.loadAd(/*"ca-app-pub-3940256099942544/5224354917"*/ RewardedVideoUnitID, new AdRequest.Builder().build());
                    }
                    catch(Exception e)
                    {

                    }
                }
            });
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
                        if (mInterstitialAd.isLoaded()) {
                            mInterstitialAd.show();
                        }
                    }
                    catch(Exception e)
                    {

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
                        if (mRewardedVideoAd.isLoaded()) {
                            mRewardedVideoAd.show();
                        }
                    }
                    catch(Exception e)
                    {

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
}