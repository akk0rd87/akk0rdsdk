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

private external fun adCallback(eventType: Int)
private external fun initCallback(code: Int)

class AdMobAdapter {
    private class MyInterstitialCallback : InterstitialAdLoadCallback() {
        override fun onAdLoaded(interstitialAd: InterstitialAd) {
            try {
                mInterstitialAd = interstitialAd
                adCallbackLocal(EVENT_INTERSTITIAL_LOADED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onAdFailedToLoad(loadAdError: LoadAdError) {
            try {
                // Handle the error
                mInterstitialAd = null
                adCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_LOAD)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }
    }

    private class MyInterstitialContentCallback : FullScreenContentCallback() {
        override fun onAdDismissedFullScreenContent() {
            try {
                // Called when fullscreen content is dismissed.
                mInterstitialAd = null
                Log.d(getTag(), "The ad was dismissed.")
                adCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onAdFailedToShowFullScreenContent(adError: AdError) {
            try {
                // Called when fullscreen content failed to show.
                mInterstitialAd = null
                Log.d(getTag(), "The ad failed to show.")
                adCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_SHOW)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onAdShowedFullScreenContent() {
            try {
                // Called when fullscreen content is shown.
                // Make sure to set your reference to null so you don't
                // show it a second time.
                mInterstitialAd = null
                // тут вызываем Closed, так как обнулили ссылку и можно запращивать новую рекламу
                adCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
                Log.d(getTag(), "The ad was shown.")
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }
    }

    private class MyRewardedCallback : RewardedAdLoadCallback(), OnUserEarnedRewardListener {
        override fun onAdFailedToLoad(loadAdError: LoadAdError) {
            try {
                // Handle the error.
                Log.d(getTag(), loadAdError.message)
                mRewardedAd = null
                adCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onAdLoaded(rewardedAd: RewardedAd) {
            try {
                mRewardedAd = rewardedAd
                adCallbackLocal(EVENT_REWARDEDVIDEO_LOADED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        //
        // OnUserEarnedRewardListener Callback
        //
        override fun onUserEarnedReward(rewardItem: RewardItem) {
            try {
                Log.d(getTag(), "The user earned the reward.")
                adCallbackLocal(EVENT_REWARDEDVIDEO_REWARDED)
                mRewardedAd = null
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }
    }

    private class MyRewardedContentCallback : FullScreenContentCallback() {
        override fun onAdShowedFullScreenContent() {
            try {
                Log.d(getTag(), "Ad was shown.")
                mRewardedAd = null
                adCallbackLocal(EVENT_REWARDEDVIDEO_STARTED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onAdFailedToShowFullScreenContent(adError: AdError) {
            try {
                // Called when ad fails to show.
                Log.d(getTag(), "Ad failed to show.")
                mRewardedAd = null
                adCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_SHOW)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onAdDismissedFullScreenContent() {
            try {
                // Called when ad is dismissed.
                // Don't forget to set the ad reference to null so you
                // don't show the ad a second time.
                mRewardedAd = null
                Log.d(getTag(), "Ad was dismissed.")
                adCallbackLocal(EVENT_REWARDEDVIDEO_CLOSED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }
    }

    companion object : OnInitializationCompleteListener {
        private val mInterstitialCallbackListener: MyInterstitialCallback by lazy { MyInterstitialCallback() }
        private val mInterstitialContentCallback: MyInterstitialContentCallback by lazy { MyInterstitialContentCallback() }
        private val mRewardedCallback: MyRewardedCallback by lazy { MyRewardedCallback() }
        private val mRewardedContentCallback: MyRewardedContentCallback by lazy { MyRewardedContentCallback() }

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
                        mAdMobInitializationCompleted = true
                        initCallback(INIT_SUCCESS)
                        return
                    }
                }
                initCallback(INIT_ERROR)
            } catch (e: Exception) {
                initCallback(INIT_ERROR)
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        private fun adCallbackLocal(eventType: Int) {
            try {
                adCallback(eventType)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun initialize() {
            try {
                mAdMobInitializationCompleted = false
                MobileAds.initialize(getContext(), this)
            } catch (e: Exception) {
                initCallback(INIT_ERROR)
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun interstitialSetUnitId(id: String) {
            try {
                InterstitialUnitID = id
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun rewardedVideoSetUnitId(id: String) {
            try {
                RewardedVideoUnitID = id
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun interstitialLoad() {
            try {
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
                            e.message?.let { Log.e(getTag(), it) }
                        }
                    })
                } else {
                    // если SDK не проинициализировано, имитируем, как будто загрузка прошла неуспешно
                    adCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_LOAD)
                }
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun rewardedVideoLoad() {
            try {
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
                            e.message?.let { Log.e(getTag(), it) }
                        }
                    })
                } else {
                    // если SDK не проинициализировано, имитурем, как будто загрузка прошла неуспешно
                    adCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD)
                }
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun interstitialShow(): Int {
            try {
                getContext().runOnUiThread(Runnable {
                    try {
                        mInterstitialAd?.fullScreenContentCallback = mInterstitialContentCallback
                        mInterstitialAd?.show(getContext())
                    } catch (e: Exception) {
                        e.message?.let { Log.e(getTag(), it) }
                    }
                })

                return 0
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
                return -1
            }
        }

        @JvmStatic
        fun rewardedVideoShow(): Int {
            try {
                getContext().runOnUiThread(Runnable {
                    try {
                        mRewardedAd?.fullScreenContentCallback = mRewardedContentCallback
                        mRewardedAd?.show(getContext(), mRewardedCallback)
                    } catch (e: Exception) {
                        e.message?.let { Log.e(getTag(), it) }
                    }
                })

                return 0
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
                return -1
            }
        }

        private fun getContext(): Activity = org.akkord.lib.Utils.GetContext()
        private fun getTag(): String = org.akkord.lib.Utils.TAG
    }
}