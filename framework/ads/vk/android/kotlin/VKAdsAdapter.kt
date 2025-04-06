package org.akkord.lib

import android.annotation.SuppressLint
import android.app.Activity
import android.util.Log
import com.my.target.ads.InterstitialAd
import com.my.target.ads.Reward
import com.my.target.ads.RewardedAd
import com.my.target.common.models.IAdLoadingError
import com.my.target.common.MyTargetManager;

private external fun adCallback(eventType: Int)
private external fun initCallback(code: Int)

class VKAdsAdapter {
    private class MyInterstitialCallback : InterstitialAd.InterstitialAdListener {
        override fun onLoad(interstitialAd: InterstitialAd) {
            try {
                mInterstitialAd = interstitialAd
                Log.d(getTag(), "mInterstitialAd: The ad loaded")
                adCallbackLocal(EVENT_INTERSTITIAL_LOADED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onNoAd(adRequestError: IAdLoadingError, interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onNoAd $adRequestError")
                adCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_LOAD)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onClick(interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onClick")
                adCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onDismiss(interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onDismiss")
                adCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onVideoCompleted(interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onVideoCompleted")
                //adCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onDisplay(interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onDisplay")
                //adCallbackLocal(EVENT_INTERSTITIAL_CLOSED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onFailedToShow(interstitialAd: InterstitialAd) {
            try {
                Log.d(getTag(), "mInterstitialAd: onFailedToShow")
                adCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_SHOW)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }
    }

    private class MyRewardedCallback :  RewardedAd.RewardedAdListener {
        override fun onLoad(rewardedAd: RewardedAd) {
            try {
                mRewardedAd = rewardedAd
                Log.d(getTag(), "MyRewardedCallback: The ad loaded")
                adCallbackLocal(EVENT_REWARDEDVIDEO_LOADED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onNoAd(adRequestError: IAdLoadingError, rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "No ad")
                adCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onClick(rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "onClick")
                adCallbackLocal(EVENT_REWARDEDVIDEO_CLOSED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onDismiss(rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "onDismiss")
                adCallbackLocal(EVENT_REWARDEDVIDEO_CLOSED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onReward(reward: Reward, rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "onReward")
                adCallbackLocal(EVENT_REWARDEDVIDEO_REWARDED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onDisplay(rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "onDisplay")
                adCallbackLocal(EVENT_REWARDEDVIDEO_STARTED)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        override fun onFailedToShow(rewardedAd: RewardedAd) {
            try {
                Log.d(getTag(), "onFailedToShow")
                adCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_SHOW)
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }
    }

    companion object {
        private val mInterstitialCallback: MyInterstitialCallback by lazy { MyInterstitialCallback() }
        private val mRewardedCallback: MyRewardedCallback by lazy { MyRewardedCallback() }

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

        @SuppressLint("StaticFieldLeak")
        private var mInterstitialAd: InterstitialAd? = null
        @SuppressLint("StaticFieldLeak")
        private var mRewardedAd: RewardedAd? = null

        private var InterstitialUnitID: Int? = null
        private var RewardedVideoUnitID: Int? = null

        private var mInitializationCompleted = false

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
                Log.d(getTag(), "INIT_VKADS")
                MyTargetManager.initSdk(getContext())
                if (MyTargetManager.isSdkInitialized()) {
                    Log.d(getTag(), "INIT_SUCCESS")
                    initCallback(INIT_SUCCESS)
                    mInitializationCompleted = true
                }
                else {
                    Log.d(getTag(), "INIT_ERROR")
                    initCallback(INIT_ERROR)
                }
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun interstitialSetUnitId(id: String) {
            try {
                Log.d(getTag(), "VKADS: interstitialSetUnitId: $id")
                InterstitialUnitID = id.toInt()
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun rewardedVideoSetUnitId(id: String) {
            try {
                RewardedVideoUnitID = id.toInt()
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
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
                    adCallbackLocal(EVENT_INTERSTITIAL_FAILED_TO_LOAD)
                }
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
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
                    adCallbackLocal(EVENT_REWARDEDVIDEO_FAILED_TO_LOAD)
                }
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
            }
        }

        @JvmStatic
        fun interstitialShow(): Int {
            Log.d(getTag(), "VKADS: interstitialShow")
            try {
                getContext().runOnUiThread(Runnable {
                    try {
                        mInterstitialAd?.show(getContext())
                    } catch (e: Exception) {
                        e.message?.let { Log.e(getTag(), it) }
                    }
                })
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
                return -1
            }
            return 0
        }

        @JvmStatic
        fun rewardedVideoShow(): Int {
            try {
                getContext().runOnUiThread(Runnable {
                    try {
                        mRewardedAd?.show(getContext())
                    } catch (e: Exception) {
                        e.message?.let { Log.e(getTag(), it) }
                    }
                })
            } catch (e: Exception) {
                e.message?.let { Log.e(getTag(), it) }
                return -1
            }
            return 0
        }

        private fun getContext(): Activity = org.akkord.lib.Utils.GetContext()
        private fun getTag(): String = org.akkord.lib.Utils.TAG
    }
}