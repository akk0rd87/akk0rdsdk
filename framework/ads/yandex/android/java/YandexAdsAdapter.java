package org.akkord.lib;

import android.app.Activity;
import android.util.Log;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.yandex.mobile.ads.common.AdError;
import com.yandex.mobile.ads.common.AdRequestConfiguration;
import com.yandex.mobile.ads.common.MobileAds;
import com.yandex.mobile.ads.common.InitializationListener;
import com.yandex.mobile.ads.common.AdRequestError;
import com.yandex.mobile.ads.common.ImpressionData;
import com.yandex.mobile.ads.interstitial.InterstitialAdLoadListener;
import com.yandex.mobile.ads.interstitial.InterstitialAdLoader;
import com.yandex.mobile.ads.rewarded.Reward;
import com.yandex.mobile.ads.rewarded.RewardedAd;
import com.yandex.mobile.ads.rewarded.RewardedAdEventListener;
import com.yandex.mobile.ads.interstitial.InterstitialAd;
import com.yandex.mobile.ads.interstitial.InterstitialAdEventListener;
import com.yandex.mobile.ads.rewarded.RewardedAdLoadListener;
import com.yandex.mobile.ads.rewarded.RewardedAdLoader;

public class YandexAdsAdapter implements InitializationListener {
    private static String TAG = "SDL";
    private static YandexAdsAdapter       yandexAdsAdapter = null;
    private static MyInterstitialManager  intManager = null;
    private static MyRewardedVideoManager rvManager = null;
    private static Activity               myActivity = null;

    private static native void AdCallback(int EventType);
    private static native void InitCallback(int Code);

    private static final int INIT_SUCCESS       = 0;
    private static final int INIT_ERROR         = 1;

    public static void Initialize(Activity activity, int interstitial, int rewardedvideo) {
        try {
            myActivity = activity;
            yandexAdsAdapter = new YandexAdsAdapter();

            MobileAds.initialize(myActivity, yandexAdsAdapter);

            if(interstitial != 0) {
                intManager       = new MyInterstitialManager();
                intManager.Init(myActivity);
            }

            if(rewardedvideo != 0) {
                rvManager        = new MyRewardedVideoManager();
                rvManager.Init(myActivity);
            }
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

    private static class MyInterstitialManager implements InterstitialAdLoadListener, InterstitialAdEventListener {
        private static final int EVENT_INTERSTITIAL_LOADED          = 1;
        private static final int EVENT_INTERSTITIAL_OPENED          = 2;
        private static final int EVENT_INTERSTITIAL_CLOSED          = 3;
        private static final int EVENT_INTERSTITIAL_FAILED_TO_LOAD  = 4;
        private static final int EVENT_INTERSTITIAL_LEFTAPPLICATION = 5;
        private static final int EVENT_INTERSTITIAL_FAILED_TO_SHOW  = 6;
        private InterstitialAd mInterstitialAd;
        private InterstitialAdLoader loader;
        private AdRequestConfiguration adRequestConfiguration = null;

        @Override
        public void onAdLoaded(@NonNull InterstitialAd interstitialAd) {
            mInterstitialAd = interstitialAd;
            AdCallback(EVENT_INTERSTITIAL_LOADED);
            Log.d(TAG, "YandexADS: onAdLoaded");
        }

        @Override
        public void onAdFailedToLoad(@NonNull AdRequestError adRequestError) {
            AdCallback(EVENT_INTERSTITIAL_FAILED_TO_LOAD);
            Log.d(TAG, "YandexADS: onAdFailedToLoad");
        }

        @Override
        public void onAdFailedToShow(@NonNull AdError adError) {
            AdCallback(EVENT_INTERSTITIAL_FAILED_TO_SHOW);
            Log.d(TAG, "YandexADS: onAdFailedToShow");
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
        public void onAdImpression(@Nullable ImpressionData impressionData) {
            AdCallback(EVENT_INTERSTITIAL_OPENED);
            Log.d(TAG, "YandexADS: onImpression");
        }

        @Override
        public void onAdClicked() {
        }

        public void InterstitialSetUnitId(final String ID) {
            Log.d(TAG, "YandexADS: InterstitialSetUnitId");
            adRequestConfiguration = new AdRequestConfiguration.Builder(ID).build();
        }

        private void Destroy() {
            try {
                if(null != mInterstitialAd) {
                    mInterstitialAd = null;
                }
            }
            catch(Exception e) {
                Log.e(TAG, e.getMessage());
            }
        }

        public void InterstitialLoad() {
            Log.d(TAG, "YandexADS: InterstitialLoad");
            Destroy();
            if(null != adRequestConfiguration) {
                loader.loadAd(adRequestConfiguration);
            }
        }

        public int InterstitialShow(Activity activity) {
            Log.d(TAG, "YandexADS: InterstitialShow");
            if(null != mInterstitialAd) {
                mInterstitialAd.setAdEventListener(this);
                mInterstitialAd.show(activity);
                return 0;
            }
            return 1;
        }

        public void Init(Activity activity) {
            loader = new InterstitialAdLoader(activity);
            loader.setAdLoadListener(this);
        }
    }

    private static class MyRewardedVideoManager implements RewardedAdLoadListener, RewardedAdEventListener {
        private static final int EVENT_REWARDEDVIDEO_LOADED          = 101;
        private static final int EVENT_REWARDEDVIDEO_OPENED          = 102;
        private static final int EVENT_REWARDEDVIDEO_CLOSED          = 103;
        private static final int EVENT_REWARDEDVIDEO_FAILED_TO_LOAD  = 104;
        private static final int EVENT_REWARDEDVIDEO_LEFTAPPLICATION = 105;
        private static final int EVENT_REWARDEDVIDEO_STARTED         = 106;
        private static final int EVENT_REWARDEDVIDEO_COMPLETED       = 107;
        private static final int EVENT_REWARDEDVIDEO_REWARDED        = 108;
        private static final int EVENT_REWARDEDVIDEO_FAILED_TO_SHOW  = 109;
        private RewardedAd mRewardedAd;
        private RewardedAdLoader loader;
        private AdRequestConfiguration adRequestConfiguration = null;

        @Override
        public void onAdLoaded(@NonNull RewardedAd rewardedAd) {
            mRewardedAd = rewardedAd;
            AdCallback(EVENT_REWARDEDVIDEO_LOADED);
        }

        @Override
        public void onRewarded(final Reward reward) {
            AdCallback(EVENT_REWARDEDVIDEO_REWARDED);
        }

        @Override
        public void onAdFailedToLoad(@NonNull AdRequestError adRequestError) {
            AdCallback(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD);
        }

        @Override
        public void onAdFailedToShow(@NonNull AdError adError) {
            AdCallback(EVENT_REWARDEDVIDEO_FAILED_TO_SHOW);
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
        public void onAdImpression(@Nullable ImpressionData impressionData) {
            AdCallback(EVENT_REWARDEDVIDEO_OPENED);
        }

        @Override
        public void onAdClicked() {
        }

        public void RewardedVideoSetUnitId(final String ID) {
            adRequestConfiguration = new AdRequestConfiguration.Builder(ID).build();
        }

        private void Destroy() {
            try {
                if(null != mRewardedAd) {
                    mRewardedAd = null;
                }
            }
            catch(Exception e) {
                Log.e(TAG, e.getMessage());
            }
        }

        public void RewardedVideoLoad() {
            Destroy();
            if(null != adRequestConfiguration) {
                loader.loadAd(adRequestConfiguration);
            }
        }

        public int RewardedVideoShow(Activity activity) {
            if(null != mRewardedAd) {
                mRewardedAd.setAdEventListener(this);
                mRewardedAd.show(activity);
                return 0;
            }
            return 1;
        }

        public void Init(Activity activity) {
            loader = new RewardedAdLoader(activity);
            loader.setAdLoadListener(this);
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
                intManager.InterstitialShow(myActivity);
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
                rvManager.RewardedVideoShow(myActivity);
            }
            return 0;
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
            return 1;
        }
    }
}