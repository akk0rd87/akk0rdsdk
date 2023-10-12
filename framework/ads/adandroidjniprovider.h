#ifndef __AKK0RD_SDK_ADS_ANDROIDPROVIDER_H__
#define __AKK0RD_SDK_ADS_ANDROIDPROVIDER_H__

#include <jni.h>
#include "adunitstatus.h"
#include "core/platformwrapper/android/android_javautf8_string.h"

namespace ads {
    class AndroidJNIProvider {
    public:
        AndroidJNIProvider(
            const char* javaClass,
            JNIEnv* jnienv,
            ads::Format format,
            InterstitialStatus& interstitialStatus,
            RewardedVideoStatus& rewardedVideoStatus
        ) :
            jniEnv(jnienv),
            adsGlobalRef(nullptr),
            midInterstitialSetUnitId(nullptr),
            midInterstitialLoad(nullptr),
            midInterstitialShow(nullptr),
            midRewardedVideoSetUnitId(nullptr),
            midRewardedVideoLoad(nullptr),
            midRewardedVideoShow(nullptr)
        {
            JNIEnv* env = getJNIEnv();
            {
                jclass adsClass = env->FindClass(javaClass);
                adsGlobalRef = (jclass)env->NewGlobalRef(adsClass);
            }

            if (!adsGlobalRef) {
                logError("Could not find class");
                return;
            }

            jmethodID initMethod = env->GetStaticMethodID(adsGlobalRef, "Initialize", "(II)V");
            if (!initMethod) {
                logError("Initialize Java method not Found");
                return;
            }

            {
                const auto interstitialInit = (!(!(format & ads::Format::Interstitial))) ? 1 : 0;
                const auto rewardedvideoInit = (!(!(format & ads::Format::RewardedVideo))) ? 1 : 0;
                env->CallStaticVoidMethod(adsGlobalRef, initMethod, interstitialInit, rewardedvideoInit);

                if (interstitialInit) {
                    interstitialStatus = ads::InterstitialStatus::ReadyToLoad;
                }

                if (rewardedvideoInit) {
                    rewardedVideoStatus = ads::RewardedVideoStatus::ReadyToLoad;
                }
            }

            midInterstitialSetUnitId = env->GetStaticMethodID(adsGlobalRef, "InterstitialSetUnitId", "(Ljava/lang/String;)V");
            midInterstitialLoad = env->GetStaticMethodID(adsGlobalRef, "InterstitialLoad", "()V");
            midInterstitialShow = env->GetStaticMethodID(adsGlobalRef, "InterstitialShow", "()I");
            midRewardedVideoSetUnitId = env->GetStaticMethodID(adsGlobalRef, "RewardedVideoSetUnitId", "(Ljava/lang/String;)V");
            midRewardedVideoLoad = env->GetStaticMethodID(adsGlobalRef, "RewardedVideoLoad", "()V");
            midRewardedVideoShow = env->GetStaticMethodID(adsGlobalRef, "RewardedVideoShow", "()I");

            if (!midInterstitialSetUnitId) { logError("midInterstitialSetUnitId Java method not found"); }
            if (!midInterstitialLoad) { logError("midInterstitialLoad Java method not found"); }
            if (!midInterstitialShow) { logError("midInterstitialShow Java method not found"); }
            if (!midRewardedVideoSetUnitId) { logError("midRewardedVideoSetUnitId Java method not found"); }
            if (!midRewardedVideoLoad) { logError("midRewardedVideoLoad Java method not found"); }
            if (!midRewardedVideoShow) { logError("midRewardedVideoShow Java method not found"); }
        }

        void InterstitialSetUnitId(const std::string& unitId) {
            JNIEnv* env = getJNIEnv();
            if (!midInterstitialSetUnitId) {
                logError("InterstitialSetUnitId Java method not Found");
                return;
            }
            AndroidJavaUTF8String url_jstring(env, unitId.c_str());
            env->CallStaticVoidMethod(adsGlobalRef, midInterstitialSetUnitId, url_jstring.get());
        }

        void setRewardedVideoUnit(const char* unit) {
            JNIEnv* env = getJNIEnv();
            if (!midRewardedVideoSetUnitId)
            {
                logError("RewardedVideoSetUnitId Java method not Found");
                return;
            }
            AndroidJavaUTF8String url_jstring(env, unit);
            env->CallStaticVoidMethod(adsGlobalRef, midRewardedVideoSetUnitId, url_jstring.get());
            return;
        }

        void v_tryLoadInterstitial() {
            JNIEnv* env = getJNIEnv();
            if (!midInterstitialLoad) {
                logError("InterstitialLoad Java method not Found");
                return;
            }
            env->CallStaticVoidMethod(adsGlobalRef, midInterstitialLoad);
        }

        void v_tryLoadRewardedVideo() {
            JNIEnv* env = getJNIEnv();
            if (!midRewardedVideoLoad) {
                logError("RewardedVideoLoad Java method not Found");
                return;
            }
            env->CallStaticVoidMethod(adsGlobalRef, midRewardedVideoLoad);
        }

        void v_showInterstitial() {
            JNIEnv* env = getJNIEnv();
            if (!midInterstitialShow) {
                logError("InterstitialShow Java method not Found");
                return;
            }
            env->CallStaticIntMethod(adsGlobalRef, midInterstitialShow);
        }

        void v_showRewardedVideo() {
            JNIEnv* env = getJNIEnv();
            if (!midRewardedVideoShow) {
                logError("RewardedVideoShow Java method not Found");
                return;
            }
            env->CallStaticIntMethod(adsGlobalRef, midRewardedVideoShow);
        }

        JNIEnv* getJNIEnv() {
            return jniEnv;
        }
    private:
        JNIEnv* jniEnv;
        jclass adsGlobalRef;
        jmethodID midInterstitialSetUnitId;
        jmethodID midInterstitialLoad;
        jmethodID midInterstitialShow;
        jmethodID midRewardedVideoSetUnitId;
        jmethodID midRewardedVideoLoad;
        jmethodID midRewardedVideoShow;
    };
}

#endif // __AKK0RD_SDK_ADS_ANDROIDPROVIDER_H__