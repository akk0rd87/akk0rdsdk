package org.akkord.lib

import android.annotation.SuppressLint
import android.app.Activity
import android.util.Log
import com.my.target.ads.InterstitialAd
import com.my.target.ads.Reward
import com.my.target.ads.RewardedAd
import com.my.target.common.models.IAdLoadingError
import com.my.target.common.MyTargetManager

private external fun adCallback(eventType: Int)
private external fun initCallback(code: Int)

class VKAdsAdapter {
    private class MyInterstitialCallback : InterstitialAd.InterstitialAdListener {
        override fun onLoad(interstitialAd: InterstitialAd) {
            try {
                mInterstitialAd = interstitialAd
                Log.d(getTag(), "mInterstitialAd: The ad loaded")
                interstitialCallbackLocal(EVENT_INTERSTITIAL_LOADED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialCallback.onLoad")
            }
        }

        override fun onNoAd(adRequestError: IAdLoadingError, interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onNoAd $adRequestError")
                interstitialCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_LOAD)
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialCallback.onNoAd")
            }
        }

        override fun onClick(interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onClick")
                interstitialCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialCallback.onClick")
            }
        }

        override fun onDismiss(interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onDismiss")
                interstitialCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialCallback.onDismiss")
            }
        }

        override fun onVideoCompleted(interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onVideoCompleted")
                //interstitialCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialCallback.onVideoCompleted")
            }
        }

        override fun onDisplay(interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onDisplay")
                //interstitialCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialCallback.onDisplay")
            }
        }

        override fun onFailedToShow(interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onFailedToShow")
                interstitialCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_SHOW)
            } catch (e: Exception) {
                Utils.handleException(e, "MyInterstitialCallback.onFailedToShow")
            }
        }
    }

    private class MyRewardedCallback : RewardedAd.RewardedAdListener {
        override fun onLoad(rewardedAd: RewardedAd) {
            try {
                mRewardedAd = rewardedAd
                Log.d(getTag(), "MyRewardedCallback: The ad loaded")
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_LOADED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedCallback.onLoad")
            }
        }

        override fun onNoAd(adRequestError: IAdLoadingError, rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "No ad")
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedCallback.onNoAd")
            }
        }

        override fun onClick(rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "onClick")
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_CLOSED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedCallback.onClick")
            }
        }

        override fun onDismiss(rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "onDismiss")
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_CLOSED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedCallback.onDismiss")
            }
        }

        override fun onReward(reward: Reward, rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "onReward")
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_REWARDED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedCallback.onReward")
            }
        }

        override fun onDisplay(rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "onDisplay")
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_STARTED)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedCallback.onDisplay")
            }
        }

        override fun onFailedToShow(rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "onFailedToShow")
                rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_SHOW)
            } catch (e: Exception) {
                Utils.handleException(e, "MyRewardedCallback.onFailedToShow")
            }
        }
    }

    companion object {
        private const val ADS_SOURCE = "ads_vk"

        private val mInterstitialCallback: MyInterstitialCallback by lazy { MyInterstitialCallback() }
        private val mRewardedCallback: MyRewardedCallback by lazy { MyRewardedCallback() }

        private const val INIT_SUCCESS = 0
        private const val INIT_ERROR = 1

        @SuppressLint("StaticFieldLeak")
        private var mInterstitialAd: InterstitialAd? = null
        @SuppressLint("StaticFieldLeak")
        private var mRewardedAd: RewardedAd? = null

        private var InterstitialUnitID: Int? = null
        private var RewardedVideoUnitID: Int? = null

        private var mInitializationCompleted = false

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
                Log.d(getTag(), "INIT_VKADS")
                MyTargetManager.initSdk(getContext())
                if (MyTargetManager.isSdkInitialized()) {
                    Log.d(getTag(), "INIT_SUCCESS")
                    initCallbackLocal(INIT_SUCCESS)
                    mInitializationCompleted = true
                }
                else {
                    Log.d(getTag(), "INIT_ERROR")
                    initCallbackLocal(INIT_ERROR)
                }
            } catch (e: Exception) {
                Utils.handleException(e, "initialize")
                initCallbackLocal(INIT_ERROR)
            }
        }

        @JvmStatic
        fun interstitialSetUnitId(id: String) {
            try {
                Log.d(getTag(), "VKADS: interstitialSetUnitId: $id")
                InterstitialUnitID = id.toInt()
            } catch (e: Exception) {
                Utils.handleException(e, "interstitialSetUnitId")
            }
        }

        @JvmStatic
        fun rewardedVideoSetUnitId(id: String) {
            try {
                RewardedVideoUnitID = id.toInt()
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoSetUnitId")
            }
        }

        @JvmStatic
        fun interstitialLoad() {
            try {
                Log.d(getTag(), "VKADS: interstitialLoad")
                if (mInitializationCompleted) {
                    getContext().runOnUiThread(Runnable {
                        InterstitialUnitID?.let { unitId ->
                            mInterstitialAd = InterstitialAd(unitId, getContext()).apply {
                                listener = mInterstitialCallback
                            }
                            mInterstitialAd?.load()
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
                if (mInitializationCompleted) {
                    getContext().runOnUiThread(Runnable {
                        RewardedVideoUnitID?.let { unitId ->
                            mRewardedAd = RewardedAd(unitId, getContext()).apply {
                                listener = mRewardedCallback
                            }
                            mRewardedAd?.load()
                        }
                    })
                } else {
                    // если SDK не проинициализировано, имитируем, как будто загрузка прошла неуспешно
                    rewardedVideoCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD)
                }
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoLoad")
            }
        }

        @JvmStatic
        fun interstitialShow(): Int {
            Log.d(getTag(), "VKADS: interstitialShow")
            try {
                Utils.logFirebaseInterstitialShow(ADS_SOURCE, InterstitialUnitID?.toString())
                getContext().runOnUiThread(Runnable {
                    try {
                        mInterstitialAd?.show(getContext())
                    } catch (e: Exception) {
                        Utils.handleException(e, "interstitialShow: runOnUiThread")
                    }
                })
            } catch (e: Exception) {
                Utils.handleException(e, "interstitialShow")
                return -1
            }
            return 0
        }

        @JvmStatic
        fun rewardedVideoShow(): Int {
            try {
                Utils.logFirebaseRewardedVideoShow(ADS_SOURCE, RewardedVideoUnitID?.toString())
                getContext().runOnUiThread(Runnable {
                    try {
                        mRewardedAd?.show(getContext())
                    } catch (e: Exception) {
                        Utils.handleException(e, "rewardedVideoShow: runOnUiThread")
                    }
                })
            } catch (e: Exception) {
                Utils.handleException(e, "rewardedVideoShow")
                return -1
            }
            return 0
        }

        private fun getContext(): Activity = org.akkord.lib.Utils.GetContext()
        private fun getTag(): String = org.akkord.lib.Utils.TAG
    }
}