package org.akkord.lib

import android.app.Activity
import android.util.Log
import com.yandex.mobile.ads.common.AdError
import com.yandex.mobile.ads.common.AdRequestConfiguration
import com.yandex.mobile.ads.common.AdRequestError
import com.yandex.mobile.ads.common.ImpressionData
import com.yandex.mobile.ads.common.InitializationListener
import com.yandex.mobile.ads.common.MobileAds
import com.yandex.mobile.ads.interstitial.InterstitialAd
import com.yandex.mobile.ads.interstitial.InterstitialAdEventListener
import com.yandex.mobile.ads.interstitial.InterstitialAdLoadListener
import com.yandex.mobile.ads.interstitial.InterstitialAdLoader
import com.yandex.mobile.ads.rewarded.Reward
import com.yandex.mobile.ads.rewarded.RewardedAd
import com.yandex.mobile.ads.rewarded.RewardedAdEventListener
import com.yandex.mobile.ads.rewarded.RewardedAdLoadListener
import com.yandex.mobile.ads.rewarded.RewardedAdLoader

private external fun adCallback(eventType: Int)
private external fun initCallback(code: Int)

class YandexAdsAdapter {
    private class MyInterstitialManager : InterstitialAdLoadListener, InterstitialAdEventListener {
        private val loader: InterstitialAdLoader by lazy { getInterstitialAdLoader() }
        private var mInterstitialAd: InterstitialAd? = null
        private var adRequestConfiguration: AdRequestConfiguration? = null

        override fun onAdLoaded(interstitialAd: InterstitialAd) {
            mInterstitialAd = interstitialAd
            adCallback(EVENT_INTERSTITIAL_LOADED)
            Log.d(getTag(), "YandexADS: onAdLoaded")
        }

        override fun onAdFailedToLoad(error: AdRequestError) {
            adCallback(EVENT_INTERSTITIAL_FAILED_TO_LOAD)
            Log.d(getTag(), "YandexADS: onAdFailedToLoad")
        }

        override fun onAdFailedToShow(adError: AdError) {
            adCallback(EVENT_INTERSTITIAL_FAILED_TO_SHOW)
            Log.d(getTag(), "YandexADS: onAdFailedToShow")
        }

        override fun onAdShown() {
            adCallback(EVENT_INTERSTITIAL_CLOSED)
            Log.d(getTag(), "YandexADS: onAdShown")
        }

        override fun onAdDismissed() {
            adCallback(EVENT_INTERSTITIAL_CLOSED)
            Log.d(getTag(), "YandexADS: onAdDismissed")
        }

        override fun onAdImpression(impressionData: ImpressionData?) {
            adCallback(EVENT_INTERSTITIAL_OPENED)
            Log.d(getTag(), "YandexADS: onImpression")
        }

        override fun onAdClicked() {
        }

        fun interstitialSetUnitId(id: String) {
            try {
                Log.d(getTag(), "YandexADS: InterstitialSetUnitId")
                adRequestConfiguration = AdRequestConfiguration.Builder(id).build()
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        private fun destroy() {
            try {
                mInterstitialAd = null
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        fun interstitialLoad() {
            try {
                Log.d(getTag(), "YandexADS: InterstitialLoad")
                destroy()
                adRequestConfiguration?.let { loader.loadAd(it) }
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        fun interstitialShow(): Int {
            try {
                Log.d(getTag(), "YandexADS: InterstitialShow")
                mInterstitialAd?.setAdEventListener(this)
                mInterstitialAd?.show(getContext())
                return 0
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
            return 1
        }

        private fun getInterstitialAdLoader() : InterstitialAdLoader {
            val interstitialAdLoader = InterstitialAdLoader(getContext())
            interstitialAdLoader.setAdLoadListener(this)
            return interstitialAdLoader
        }
    }

    private class MyRewardedVideoManager : RewardedAdLoadListener, RewardedAdEventListener {
        private val loader: RewardedAdLoader by lazy { getRewardedAdLoader() }
        private var mRewardedAd: RewardedAd? = null
        private var adRequestConfiguration: AdRequestConfiguration? = null

        override fun onAdLoaded(rewarded: RewardedAd) {
            mRewardedAd = rewarded
            adCallback(EVENT_REWARDEDVIDEO_LOADED)
        }

        override fun onRewarded(reward: Reward) {
            adCallback(EVENT_REWARDEDVIDEO_REWARDED)
        }

        override fun onAdFailedToLoad(error: AdRequestError) {
            adCallback(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD)
        }

        override fun onAdFailedToShow(adError: AdError) {
            adCallback(EVENT_REWARDEDVIDEO_FAILED_TO_SHOW)
        }

        override fun onAdShown() {
            adCallback(EVENT_REWARDEDVIDEO_CLOSED)
        }

        override fun onAdDismissed() {
            adCallback(EVENT_REWARDEDVIDEO_CLOSED)
        }

        override fun onAdImpression(impressionData: ImpressionData?) {
            adCallback(EVENT_REWARDEDVIDEO_OPENED)
        }

        override fun onAdClicked() {
        }

        fun rewardedVideoSetUnitId(id: String) {
            try {
                adRequestConfiguration = AdRequestConfiguration.Builder(id).build()
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        private fun destroy() {
            try {
                mRewardedAd = null
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        fun rewardedVideoLoad() {
            try {
                destroy()
                adRequestConfiguration?.let { loader.loadAd(it) }
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        fun rewardedVideoShow(): Int {
            try {
                mRewardedAd?.setAdEventListener(this)
                mRewardedAd?.show(getContext())
                return 0
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
            return 1
        }

        private fun getRewardedAdLoader() : RewardedAdLoader {
            val rewardedAdLoader = RewardedAdLoader(getContext())
            rewardedAdLoader.setAdLoadListener(this)
            return rewardedAdLoader
        }
    }

    companion object : InitializationListener {
        private val intManager: MyInterstitialManager by lazy { MyInterstitialManager() }
        private val rvManager: MyRewardedVideoManager by lazy { MyRewardedVideoManager() }

        private const val INIT_SUCCESS = 0
        private const val INIT_ERROR = 1

        private const val EVENT_INTERSTITIAL_LOADED = 1
        private const val EVENT_INTERSTITIAL_OPENED = 2
        private const val EVENT_INTERSTITIAL_CLOSED = 3
        private const val EVENT_INTERSTITIAL_FAILED_TO_LOAD = 4
        private const val EVENT_INTERSTITIAL_LEFTAPPLICATION = 5
        private const val EVENT_INTERSTITIAL_FAILED_TO_SHOW = 6

        private const val EVENT_REWARDEDVIDEO_LOADED = 101
        private const val EVENT_REWARDEDVIDEO_OPENED = 102
        private const val EVENT_REWARDEDVIDEO_CLOSED = 103
        private const val EVENT_REWARDEDVIDEO_FAILED_TO_LOAD = 104
        private const val EVENT_REWARDEDVIDEO_LEFTAPPLICATION = 105
        private const val EVENT_REWARDEDVIDEO_STARTED = 106
        private const val EVENT_REWARDEDVIDEO_COMPLETED = 107
        private const val EVENT_REWARDEDVIDEO_REWARDED = 108
        private const val EVENT_REWARDEDVIDEO_FAILED_TO_SHOW = 109

        // InitializationListener
        override fun onInitializationCompleted() {
            Log.d(getTag(), "Yandex ads onInitializationCompleted")
            initCallback(INIT_SUCCESS)
        }

        ////////////////////////
        // Interface methods
        ////////////////////////
        @JvmStatic
        fun initialize() {
            try {
                MobileAds.initialize(getContext(), this)
            } catch (e: Exception) {
                initCallback(INIT_ERROR)
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun interstitialSetUnitId(id: String) {
            try {
                intManager.interstitialSetUnitId(id)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun rewardedVideoSetUnitId(id: String) {
            try {
                rvManager.rewardedVideoSetUnitId(id)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun interstitialLoad() {
            try {
                intManager.interstitialLoad()
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun rewardedVideoLoad() {
            try {
                rvManager.rewardedVideoLoad()
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun interstitialShow(): Int {
            try {
                intManager.interstitialShow()
                return 0
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
                return 1
            }
        }

        @JvmStatic
        fun rewardedVideoShow(): Int {
            try {
                rvManager.rewardedVideoShow()
                return 0
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
                return 1
            }
        }

        private fun getContext(): Activity = org.akkord.lib.Utils.GetContext()
        private fun getTag(): String = org.akkord.lib.Utils.TAG
    }
}