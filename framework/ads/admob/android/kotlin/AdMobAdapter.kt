package org.akkord.lib

import android.app.Activity
import android.util.Log
import com.google.android.gms.ads.AdError
import com.google.android.gms.ads.AdRequest
import com.google.android.gms.ads.FullScreenContentCallback
import com.google.android.gms.ads.LoadAdError
import com.google.android.gms.ads.MobileAds
import com.google.android.gms.ads.OnUserEarnedRewardListener
import com.google.android.gms.ads.initialization.AdapterStatus
import com.google.android.gms.ads.initialization.InitializationStatus
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener
import com.google.android.gms.ads.interstitial.InterstitialAd
import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback
import com.google.android.gms.ads.rewarded.RewardItem
import com.google.android.gms.ads.rewarded.RewardedAd
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

private external fun adCallback(eventType: Int)
private external fun initCallback(code: Int)

class AdMobAdapter {
    private class MyInterstitialCallback : InterstitialAdLoadCallback() {
        override fun onAdLoaded(interstitialAd: InterstitialAd) {
            try {
                mInterstitialAd = interstitialAd
                Log.d(getTag(), "AdMob: onAdLoaded")
                interstitialCallbackLocal(EVENT_INTERSTITIAL_LOADED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialCallback.onAdLoaded")
            }
        }

        override fun onAdFailedToLoad(loadAdError: LoadAdError) {
            try {
                // Handle the error
                mInterstitialAd = null
                Log.d(getTag(), "AdMob: onAdFailedToLoad")
                interstitialCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_LOAD)
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialCallback.onAdFailedToLoad")
            }
        }
    }

    private class MyInterstitialContentCallback : FullScreenContentCallback() {
        override fun onAdDismissedFullScreenContent() {
            try {
                // Called when fullscreen content is dismissed.
                mInterstitialAd = null
                Log.d(getTag(), "AdMob: onAdDismissedFullScreenContent")
                interstitialCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialContentCallback.onAdDismissedFullScreenContent")
            }
        }

        override fun onAdFailedToShowFullScreenContent(adError: AdError) {
            try {
                // Called when fullscreen content failed to show.
                mInterstitialAd = null
                Log.d(getTag(), "AdMob: onAdFailedToShowFullScreenContent")
                interstitialCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_SHOW)
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialContentCallback.onAdFailedToShowFullScreenContent")
            }
        }

        override fun onAdShowedFullScreenContent() {
            try {
                // Called when fullscreen content is shown.
                // Make sure to set your reference to null so you don't
                // show it a second time.
                mInterstitialAd = null
                // тут вызываем Closed, так как обнулили ссылку и можно запращивать новую рекламу
                interstitialCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
                Log.d(getTag(), "AdMob: onAdShowedFullScreenContent")
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialContentCallback.onAdShowedFullScreenContent")
            }
        }
    }

    private class MyRewardedCallback : RewardedAdLoadCallback(), OnUserEarnedRewardListener {
        override fun onAdFailedToLoad(loadAdError: LoadAdError) {
            try {
                // Handle the error.
                Log.d(getTag(), "AdMob: onAdFailedToLoad: ${loadAdError.message}")
                mRewardedAd = null
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedCallback.onAdFailedToLoad")
            }
        }

        override fun onAdLoaded(rewardedAd: RewardedAd) {
            try {
                mRewardedAd = rewardedAd
                Log.d(getTag(), "AdMob: onAdLoaded")
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_LOADED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedCallback.onAdLoaded")
            }
        }

        //
        // OnUserEarnedRewardListener Callback
        //
        override fun onUserEarnedReward(rewardItem: RewardItem) {
            try {
                Log.d(getTag(), "AdMob: onUserEarnedReward")
                Utils.logFirebaseRewardedVideoRewarded(ADS_SOURCE, RewardedVideoUnitID)
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_REWARDED)
                mRewardedAd = null
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedCallback.onUserEarnedReward")
            }
        }
    }

    private class MyRewardedContentCallback : FullScreenContentCallback() {
        override fun onAdShowedFullScreenContent() {
            try {
                Log.d(getTag(), "AdMob: onAdShowedFullScreenContent")
                mRewardedAd = null
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_STARTED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedContentCallback.onAdShowedFullScreenContent")
            }
        }

        override fun onAdFailedToShowFullScreenContent(adError: AdError) {
            try {
                // Called when ad fails to show.
                Log.d(getTag(), "AdMob: onAdFailedToShowFullScreenContent")
                mRewardedAd = null
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_SHOW)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedContentCallback.onAdFailedToShowFullScreenContent")
            }
        }

        override fun onAdDismissedFullScreenContent() {
            try {
                // Called when ad is dismissed.
                // Don't forget to set the ad reference to null so you
                // don't show the ad a second time.
                mRewardedAd = null
                Log.d(getTag(), "AdMob: onAdDismissedFullScreenContent")
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_CLOSED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedContentCallback.onAdDismissedFullScreenContent")
            }
        }
    }

    companion object : OnInitializationCompleteListener {
        private const val ADS_SOURCE = "ads_admob"

        private val mInterstitialCallbackListener: MyInterstitialCallback by lazy { MyInterstitialCallback() }
        private val mInterstitialContentCallback: MyInterstitialContentCallback by lazy { MyInterstitialContentCallback() }
        private val mRewardedCallback: MyRewardedCallback by lazy { MyRewardedCallback() }
        private val mRewardedContentCallback: MyRewardedContentCallback by lazy { MyRewardedContentCallback() }

        private const val INIT_SUCCESS = 0
        private const val INIT_ERROR = 1

        private var mInterstitialAd: InterstitialAd? = null
        private var mRewardedAd: RewardedAd? = null

        private var InterstitialUnitID: String? = null
        private var RewardedVideoUnitID: String? = null

        private var mAdMobInitializationCompleted = false

        // CALLBACK FOR OnInitializationCompleteListener
        override fun onInitializationComplete(initializationStatus: InitializationStatus) {
            try {
                val mp = initializationStatus.adapterStatusMap
                for ((_, value) in mp) {
                    if (AdapterStatus.State.READY == value.initializationState) {
                        Log.d(getTag(), "AdMob: onInitializationComplete: Success")
                        mAdMobInitializationCompleted = true
                        initCallbackLocal(INIT_SUCCESS)
                        return
                    }
                }
                initCallbackLocal(INIT_ERROR)
                Log.d(getTag(), "AdMob: onInitializationComplete: Error")
            } catch (e: Exception) {
                Utils.handleException(e, "onInitializationComplete")
                initCallbackLocal(INIT_ERROR)
            }
        }

        private fun adCallbackLocal(eventType: Int, format: AdFormat) {
            try {
                adCallback(eventType)
            } catch (e: Exception) {
                Utils.handleException(e, "adCallbackLocal")
            } catch (e: UnsatisfiedLinkError) {
                Utils.handleException(e, "adCallbackLocal")
            }
            try {
                AdsEventsListener.onAdEvent?.invoke(format, eventType)
            } catch (e: Exception) {
                Utils.handleException(e, "adCallbackLocal onAdEvent")
            } catch (e: UnsatisfiedLinkError) {
                Utils.handleException(e, "adCallbackLocal onAdEvent")
            }
        }

        private fun interstitialCallbackLocal(eventType: Int) = adCallbackLocal(eventType, AdFormat.Interstitial)
        private fun rewardedVideoCallbackLocal(eventType: Int) = adCallbackLocal(eventType, AdFormat.RewardedVideo)

        private fun initCallbackLocal(code: Int) {
            try {
                initCallback(code)
            } catch (e: Exception) {
                Utils.handleException(e, "initCallbackLocal")
            } catch (e: UnsatisfiedLinkError) {
                Utils.handleException(e, "initCallbackLocal")
            }
        }

        @JvmStatic
        fun initialize() {
            try {
                Log.d(getTag(), "AdMob: initialize")
                val backgroundScope = CoroutineScope(Dispatchers.IO)
                val initializationListener = this
                backgroundScope.launch {
                    mAdMobInitializationCompleted = false
                    MobileAds.initialize(getContext(), initializationListener)
                }
            } catch (e: Exception) {
                Utils.handleException(e, "initialize")
                initCallbackLocal(INIT_ERROR)
            }
        }

        @JvmStatic
        fun interstitialSetUnitId(id: String) {
            try {
                Log.d(getTag(), "AdMob: interstitialSetUnitId")
                InterstitialUnitID = id
            } catch (e: Exception) {
                Utils.handleException(e, "interstitialSetUnitId")
            }
        }

        @JvmStatic
        fun rewardedVideoSetUnitId(id: String) {
            try {
                Log.d(getTag(), "AdMob: rewardedVideoSetUnitId")
                RewardedVideoUnitID = id
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoSetUnitId")
            }
        }

        @JvmStatic
        fun interstitialLoad() {
            try {
                Log.d(getTag(), "AdMob: interstitialLoad")
                mInterstitialAd = null

                if (mAdMobInitializationCompleted) {
                    getContext().runOnUiThread(Runnable {
                        try {
                            InterstitialUnitID?.let { unitId ->
                                InterstitialAd.load(
                                    getContext(),
                                    unitId,
                                    AdRequest.Builder().build(),
                                    mInterstitialCallbackListener
                                )
                            }
                        } catch (e: Exception) {
                            Utils.handleException(e, "interstitialLoad: runOnUiThread")
                        }
                    })
                } else {
                    // если SDK не проинициализировано, имитируем, как будто загрузка прошла неуспешно
                    interstitialCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_LOAD)
                }
            } catch (e: Exception) {
                Utils.handleException(e, "interstitialLoad")
            }
        }

        @JvmStatic
        fun rewardedVideoLoad() {
            try {
                Log.d(getTag(), "AdMob: rewardedVideoLoad")
                mRewardedAd = null

                if (mAdMobInitializationCompleted) {
                    getContext().runOnUiThread(Runnable {
                        try {
                            RewardedVideoUnitID?.let { unitId ->
                                RewardedAd.load(
                                    getContext(),
                                    unitId,
                                    AdRequest.Builder().build(),
                                    mRewardedCallback
                                )
                            }
                        } catch (e: Exception) {
                            Utils.handleException(e, "rewardedVideoLoad: runOnUiThread")
                        }
                    })
                } else {
                    // если SDK не проинициализировано, имитурем, как будто загрузка прошла неуспешно
                    rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD)
                }
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoLoad")
            }
        }

        @JvmStatic
        fun interstitialShow(): Int {
            try {
                Log.d(getTag(), "AdMob: interstitialShow")
                Utils.logFirebaseInterstitialShow(ADS_SOURCE, InterstitialUnitID)
                getContext().runOnUiThread(Runnable {
                    try {
                        mInterstitialAd?.fullScreenContentCallback = mInterstitialContentCallback
                        mInterstitialAd?.show(getContext())
                    } catch (e: Exception) {
                        Utils.handleException(e, "interstitialShow: runOnUiThread")
                    }
                })

                return 0
            } catch (e: Exception) {
                Utils.handleException(e, "interstitialShow")
                return -1
            }
        }

        @JvmStatic
        fun rewardedVideoShow(): Int {
            try {
                Log.d(getTag(), "AdMob: rewardedVideoShow")
                Utils.logFirebaseRewardedVideoShow(ADS_SOURCE, RewardedVideoUnitID)
                getContext().runOnUiThread(Runnable {
                    try {
                        mRewardedAd?.fullScreenContentCallback = mRewardedContentCallback
                        mRewardedAd?.show(getContext(), mRewardedCallback)
                    } catch (e: Exception) {
                        Utils.handleException(e, "rewardedVideoShow: runOnUiThread")
                    }
                })

                return 0
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoShow")
                return -1
            }
        }

        private fun getContext(): Activity = org.akkord.lib.Utils.GetContext()
        private fun getTag(): String = org.akkord.lib.Utils.TAG
    }
}