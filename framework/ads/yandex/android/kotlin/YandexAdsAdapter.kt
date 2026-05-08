package org.akkord.lib

import android.app.Activity
import android.util.Log
import com.yandex.mobile.ads.common.AdError
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import com.yandex.mobile.ads.common.AdRequest
import com.yandex.mobile.ads.common.AdRequestError
import com.yandex.mobile.ads.common.ImpressionData
import com.yandex.mobile.ads.common.InitializationListener
import com.yandex.mobile.ads.common.YandexAds
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

private fun initCallbackLocal(code: Int) {
    try {
        initCallback(code)
    } catch (e: Exception) {
        Utils.handleException(e, "initCallbackLocal")
    } catch (e: UnsatisfiedLinkError) {
        Utils.handleException(e, "initCallbackLocal")
    }
}

private fun adCallbackLocal(eventType: Int) {
    try {
        adCallback(eventType)
    } catch (e: Exception) {
        Utils.handleException(e, "adCallbackLocal")
    } catch (e: UnsatisfiedLinkError) {
        Utils.handleException(e, "adCallbackLocal")
    }
}

class YandexAdsAdapter {
    private class MyInterstitialManager : InterstitialAdLoadListener, InterstitialAdEventListener {
        private val loader: InterstitialAdLoader by lazy { InterstitialAdLoader(getContext()) }
        private var mInterstitialAd: InterstitialAd? = null
        private var adRequestConfiguration: AdRequest? = null
        private var interstitialUnitId: String? = null

        override fun onAdLoaded(interstitialAd: InterstitialAd) {
            mInterstitialAd = interstitialAd
            adCallbackLocal(EVENT_INTERSTITIAL_LOADED)
            Log.d(getTag(), "YandexADS: onAdLoaded")
        }

        override fun onAdFailedToLoad(error: AdRequestError) {
            adCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_LOAD)
            Log.d(getTag(), "YandexADS: onAdFailedToLoad")
        }

        override fun onAdFailedToShow(adError: AdError) {
            adCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_SHOW)
            Log.d(getTag(), "YandexADS: onAdFailedToShow")
        }

        override fun onAdShown() {
            adCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            Log.d(getTag(), "YandexADS: onAdShown")
        }

        override fun onAdDismissed() {
            adCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            Log.d(getTag(), "YandexADS: onAdDismissed")
        }

        override fun onAdImpression(impressionData: ImpressionData?) {
            adCallbackLocal(EVENT_INTERSTITIAL_OPENED)
            Log.d(getTag(), "YandexADS: onImpression")
        }

        override fun onAdClicked() {
        }

        fun interstitialSetUnitId(id: String) {
            try {
                Log.d(getTag(), "YandexADS: InterstitialSetUnitId")
                interstitialUnitId = id
                adRequestConfiguration = AdRequest.Builder(id).build()
            } catch (e: Exception) {
                Utils.handleException(e, "interstitialSetUnitId")
            }
        }

        private fun destroy() {
            try {
                mInterstitialAd = null
            } catch (e: Exception) {
                Utils.handleException(e, "destroy")
            }
        }

        fun interstitialLoad() {
            try {
                Log.d(getTag(), "YandexADS: InterstitialLoad")
                destroy()
                adRequestConfiguration?.let { config ->
                    CoroutineScope(Dispatchers.Main).launch {
                        loader.loadAd(config, this@MyInterstitialManager)
                    }
                }
            } catch (e: Exception) {
                Utils.handleException(e, "interstitialLoad")
            }
        }

        fun interstitialShow(): Int {
            try {
                Log.d(getTag(), "YandexADS: InterstitialShow")
                Utils.logFirebaseInterstitialShow(ADS_SOURCE, interstitialUnitId)
                mInterstitialAd?.setAdEventListener(this)
                mInterstitialAd?.show(getContext())
                return 0
            } catch (e: Exception) {
                Utils.handleException(e, "interstitialShow")
            }
            return 1
        }
    }

    private class MyRewardedVideoManager : RewardedAdLoadListener, RewardedAdEventListener {
        private val loader: RewardedAdLoader by lazy { RewardedAdLoader(getContext()) }
        private var mRewardedAd: RewardedAd? = null
        private var adRequestConfiguration: AdRequest? = null
        private var rewardedVideoUnitId: String? = null

        override fun onAdLoaded(rewarded: RewardedAd) {
            mRewardedAd = rewarded
            adCallbackLocal(EVENT_REWARDEDVIDEO_LOADED)
        }

        override fun onRewarded(reward: Reward) {
            adCallbackLocal(EVENT_REWARDEDVIDEO_REWARDED)
        }

        override fun onAdFailedToLoad(error: AdRequestError) {
            adCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD)
        }

        override fun onAdFailedToShow(adError: AdError) {
            adCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_SHOW)
        }

        override fun onAdShown() {
            adCallbackLocal(EVENT_REWARDEDVIDEO_CLOSED)
        }

        override fun onAdDismissed() {
            adCallbackLocal(EVENT_REWARDEDVIDEO_CLOSED)
        }

        override fun onAdImpression(impressionData: ImpressionData?) {
            adCallbackLocal(EVENT_REWARDEDVIDEO_OPENED)
        }

        override fun onAdClicked() {
        }

        fun rewardedVideoSetUnitId(id: String) {
            try {
                rewardedVideoUnitId = id
                adRequestConfiguration = AdRequest.Builder(id).build()
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoSetUnitId")
            }
        }

        private fun destroy() {
            try {
                mRewardedAd = null
            } catch (e: Exception) {
                Utils.handleException(e, "destroy")
            }
        }

        fun rewardedVideoLoad() {
            try {
                destroy()
                adRequestConfiguration?.let { config ->
                    CoroutineScope(Dispatchers.Main).launch {
                        loader.loadAd(config, this@MyRewardedVideoManager)
                    }
                }
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoLoad")
            }
        }

        fun rewardedVideoShow(): Int {
            try {
                Utils.logFirebaseRewardedVideoShow(ADS_SOURCE, rewardedVideoUnitId)
                mRewardedAd?.setAdEventListener(this)
                mRewardedAd?.show(getContext())
                return 0
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoShow")
            }
            return 1
        }
    }

    companion object : InitializationListener {
        private const val ADS_SOURCE = "ads_yandex"

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
            initCallbackLocal(INIT_SUCCESS)
        }

        ////////////////////////
        // Interface methods
        ////////////////////////
        @JvmStatic
        fun initialize() {
            try {
                YandexAds.initialize(getContext(), this)
            } catch (e: Exception) {
                Utils.handleException(e, "initialize")
                initCallbackLocal(INIT_ERROR)
            }
        }

        @JvmStatic
        fun interstitialSetUnitId(id: String) {
            try {
                intManager.interstitialSetUnitId(id)
            } catch (e: Exception) {
                Utils.handleException(e, "interstitialSetUnitId")
            }
        }

        @JvmStatic
        fun rewardedVideoSetUnitId(id: String) {
            try {
                rvManager.rewardedVideoSetUnitId(id)
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoSetUnitId")
            }
        }

        @JvmStatic
        fun interstitialLoad() {
            try {
                intManager.interstitialLoad()
            } catch (e: Exception) {
                Utils.handleException(e, "interstitialLoad")
            }
        }

        @JvmStatic
        fun rewardedVideoLoad() {
            try {
                rvManager.rewardedVideoLoad()
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoLoad")
            }
        }

        @JvmStatic
        fun interstitialShow(): Int {
            try {
                intManager.interstitialShow()
                return 0
            } catch (e: Exception) {
                Utils.handleException(e, "interstitialShow")
                return 1
            }
        }

        @JvmStatic
        fun rewardedVideoShow(): Int {
            try {
                rvManager.rewardedVideoShow()
                return 0
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoShow")
                return 1
            }
        }

        private fun getContext(): Activity = org.akkord.lib.Utils.GetContext()
        private fun getTag(): String = org.akkord.lib.Utils.TAG
    }
}