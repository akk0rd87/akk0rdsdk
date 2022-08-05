package org.akkord.lib;

import android.app.Activity;
import android.content.Intent;

import android.util.Log;

import com.yandex.mobile.ads.common.MobileAds;
import com.yandex.mobile.ads.common.InitializationListener;
import com.yandex.mobile.ads.common.AdRequest;
import com.yandex.mobile.ads.common.AdRequestError;
import com.yandex.mobile.ads.common.ImpressionData;
import com.yandex.mobile.ads.rewarded.Reward;
import com.yandex.mobile.ads.rewarded.RewardedAd;
import com.yandex.mobile.ads.rewarded.RewardedAdEventListener;
import com.yandex.mobile.ads.interstitial.InterstitialAd;
import com.yandex.mobile.ads.interstitial.InterstitialAdEventListener;

public class YandexAdsAdapter implements InitializationListener {
    private static String TAG = "SDL";
    private static YandexAdsAdapter       yandexAdsAdapter = null;
    private static MyInterstitialManager  intManager = null;
    private static MyRewardedVideoManager rvManager = null;

    private static native void AdCallback(int EventType);
    private static native void InitCallback(int Code);

    private static final int INIT_SUCCESS       = 0;
    private static final int INIT_ERROR         = 1;

    public static void Initialize(int interstitial, int rewardedvideo) {
        try {
            yandexAdsAdapter = new YandexAdsAdapter();

            if(interstitial != 0) {
                intManager       = new MyInterstitialManager();
            }

            if(rewardedvideo != 0) {
                rvManager        = new MyRewardedVideoManager();
            }
            MobileAds.initialize(Utils.GetContext(), yandexAdsAdapter);
        }
        catch(Exception e) {
            InitCallback(INIT_ERROR);
            Log.e(TAG, e.getMessage());
        }
    }

    @Override // InitializationListener
    public void onInitializationCompleted() {
        Log.d(TAG, "Yandex ads onInitializationCompleted");
       InitCallback(INIT_SUCCESS);
    }

    private static class MyInterstitialManager implements InterstitialAdEventListener {
        private static final int EVENT_INTERSTITIAL_LOADED          = 1;
        private static final int EVENT_INTERSTITIAL_OPENED          = 2;
        private static final int EVENT_INTERSTITIAL_CLOSED          = 3;
        private static final int EVENT_INTERSTITIAL_FAILED_TO_LOAD  = 4;
        private static final int EVENT_INTERSTITIAL_LEFTAPPLICATION = 5;
        private static final int EVENT_INTERSTITIAL_FAILED_TO_SHOW  = 6;
        private String InterstitialUnitID;
        private InterstitialAd mInterstitialAd;

        @Override
        public void onAdLoaded() {
            AdCallback(EVENT_INTERSTITIAL_LOADED);
            Log.d(TAG, "YandexADS: onAdLoaded");
        }

        @Override
        public void onAdFailedToLoad(AdRequestError adRequestError) {
            AdCallback(EVENT_INTERSTITIAL_FAILED_TO_LOAD);
            Log.d(TAG, "YandexADS: onAdFailedToLoad");
        }

        @Override
        public void onAdShown() {
            AdCallback(EVENT_INTERSTITIAL_CLOSED);
            Log.d(TAG, "YandexADS: onAdShown");
        }

        @Override
        public void onAdDismissed() {
            AdCallback(EVENT_INTERSTITIAL_CLOSED);
            Log.d(TAG, "YandexADS: onAdDismissed");
        }

        @Override
        public void onLeftApplication() {
            AdCallback(EVENT_INTERSTITIAL_LEFTAPPLICATION);
            Log.d(TAG, "YandexADS: onLeftApplication");
        }

        @Override
        public void onReturnedToApplication() {
            Log.d(TAG, "YandexADS: onReturnedToApplication");
        }

        @Override
        public void onImpression(final ImpressionData id) {
            AdCallback(EVENT_INTERSTITIAL_OPENED);
            Log.d(TAG, "YandexADS: onImpression");
        }

        @Override
        public void onAdClicked() {
        }

        public void InterstitialSetUnitId(final String ID) {
            Log.d(TAG, "YandexADS: InterstitialSetUnitId");
            InterstitialUnitID = ID;
        }

        private void Destroy() {
            try {
                if(null != mInterstitialAd) {
                    mInterstitialAd.destroy();
                }
            }
            catch(Exception e) {
                Log.e(TAG, e.getMessage());
            }
        }

        public void InterstitialLoad() {
            Log.d(TAG, "YandexADS: InterstitialLoad");
            Destroy();

            // Creating an InterstitialAd instance.
            mInterstitialAd = new InterstitialAd(Utils.GetContext());
            mInterstitialAd.setAdUnitId(InterstitialUnitID);

            // Registering a listener to track events in the ad.
            mInterstitialAd.setInterstitialAdEventListener(this);

            // Creating an ad targeting object.
            final AdRequest adRequest = new AdRequest.Builder().build();

            // Loading ads.
            mInterstitialAd.loadAd(adRequest);
        }

        public int InterstitialShow() {
            Log.d(TAG, "YandexADS: InterstitialShow");
            if(null != mInterstitialAd) {
                if(mInterstitialAd.isLoaded()) {
                    mInterstitialAd.show();
                }
                return 0;
            }
            return 1;
        }
    }

    private static class MyRewardedVideoManager implements RewardedAdEventListener {
        private static final int EVENT_REWARDEDVIDEO_LOADED          = 101;
        private static final int EVENT_REWARDEDVIDEO_OPENED          = 102;
        private static final int EVENT_REWARDEDVIDEO_CLOSED          = 103;
        private static final int EVENT_REWARDEDVIDEO_FAILED_TO_LOAD  = 104;
        private static final int EVENT_REWARDEDVIDEO_LEFTAPPLICATION = 105;
        private static final int EVENT_REWARDEDVIDEO_STARTED         = 106;
        private static final int EVENT_REWARDEDVIDEO_COMPLETED       = 107;
        private static final int EVENT_REWARDEDVIDEO_REWARDED        = 108;
        private static final int EVENT_REWARDEDVIDEO_FAILED_TO_SHOW  = 109;
        private String RewardedVideoUnitID;
        private RewardedAd mRewardedAd;

        @Override
        public void onAdLoaded() {
            AdCallback(EVENT_REWARDEDVIDEO_LOADED);
        }

        @Override
        public void onRewarded(final Reward reward) {
            AdCallback(EVENT_REWARDEDVIDEO_REWARDED);
        }

        @Override
        public void onAdFailedToLoad(final AdRequestError adRequestError) {
            AdCallback(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD);
        }

        @Override
        public void onAdShown() {
            AdCallback(EVENT_REWARDEDVIDEO_CLOSED);
        }

        @Override
        public void onAdDismissed() {
             AdCallback(EVENT_REWARDEDVIDEO_CLOSED);
        }

        @Override
        public void onLeftApplication() {
             AdCallback(EVENT_REWARDEDVIDEO_LEFTAPPLICATION);
        }

        @Override
        public void onReturnedToApplication() {
        }

        @Override
        public void onImpression(final ImpressionData id) {
            AdCallback(EVENT_REWARDEDVIDEO_OPENED);
        }

        @Override
        public void onAdClicked() {
        }

        public void RewardedVideoSetUnitId(final String ID) {
            RewardedVideoUnitID = ID;
        }

        private void Destroy() {
            try {
                if(null != mRewardedAd) {
                    mRewardedAd.destroy();
                }
            }
            catch(Exception e) {
                Log.e(TAG, e.getMessage());
            }
        }

        public void RewardedVideoLoad() {
            Destroy();

            // Creating a RewardedAd instance.
            mRewardedAd = new RewardedAd(Utils.GetContext());
            mRewardedAd.setAdUnitId(RewardedVideoUnitID);

            // Registering a listener to track events in the ad.
            mRewardedAd.setRewardedAdEventListener(this);

            // Creating an ad targeting object.
            final AdRequest adRequest = new AdRequest.Builder().build();

            // Loading ads.
            mRewardedAd.loadAd(adRequest);
        }

        public int RewardedVideoShow() {
            if(null != mRewardedAd) {
                if(mRewardedAd.isLoaded()) {
                    mRewardedAd.show();
                    return 0;
                }
            }
            return 1;
        }
    }

    ////////////////////////
    // Interface methods
    ////////////////////////
    public static void InterstitialSetUnitId(final String ID) {
        try {
            if(null != intManager) {
                intManager.InterstitialSetUnitId(ID);
            }
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void RewardedVideoSetUnitId(final String ID) {
        try {
            if(null != rvManager) {
                rvManager.RewardedVideoSetUnitId(ID);
            }
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void InterstitialLoad() {
        try {
            if(null != intManager) {
                intManager.InterstitialLoad();
            }
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void RewardedVideoLoad() {
        try {
            if(null != rvManager) {
                rvManager.RewardedVideoLoad();
            }
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    public static int InterstitialShow() {
        try {
            if(null != intManager) {
                intManager.InterstitialShow();
            }
            return 0;
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
            return 1;
        }
    }

    public static int RewardedVideoShow() {
        try {
            if(null != rvManager) {
                rvManager.RewardedVideoShow();
            }
            return 0;
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
            return 1;
        }
    }
}