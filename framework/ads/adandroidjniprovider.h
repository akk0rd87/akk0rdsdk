#ifndef __AKK0RD_SDK_ADS_ANDROIDPROVIDER_H__
#define __AKK0RD_SDK_ADS_ANDROIDPROVIDER_H__

#include <jni.h>
#include "adunitstatus.h"

namespace ads {
    class AndroidJNIProvider {
    public:
        AndroidJNIProvider(
            const char* javaClass,
            JNIEnv* jnienv,
            jobject activity,
            ads::Format format,
            InterstitialStatus& interstitialStatus,
            RewardedVideoStatus& rewardedVideoStatus
        ) :
            jniEnv(jnienv),
            adsClass(nullptr),
            midInterstitialSetUnitId(nullptr),
            midInterstitialLoad(nullptr),
            midInterstitialShow(nullptr),
            midRewardedVideoSetUnitId(nullptr),
            midRewardedVideoLoad(nullptr),
            midRewardedVideoShow(nullptr)
        {
            JNIEnv* env = getJNIEnv();
            adsClass = env->FindClass(javaClass);

            if (!adsClass) {
                logError("Could not find class");
                return;
            }

            jmethodID initMethod = env->GetStaticMethodID(adsClass, "Initialize", "(Landroid/app/Activity;II)V");
            if (!initMethod) {
                logError("Initialize Java method not Found");
                return;
            }

            {
                const auto interstitialInit = (!(!(format & ads::Format::Interstitial))) ? 1 : 0;
                const auto rewardedvideoInit = (!(!(format & ads::Format::RewardedVideo))) ? 1 : 0;
                env->CallStaticVoidMethod(adsClass, initMethod, activity, interstitialInit, rewardedvideoInit);

                if (interstitialInit) {
                    interstitialStatus = ads::InterstitialStatus::ReadyToLoad;
                }

                if (rewardedvideoInit) {
                    rewardedVideoStatus = ads::RewardedVideoStatus::ReadyToLoad;
                }
            }

            midInterstitialSetUnitId = env->GetStaticMethodID(adsClass, "InterstitialSetUnitId", "(Ljava/lang/String;)V");
            midInterstitialLoad = env->GetStaticMethodID(adsClass, "InterstitialLoad", "()V");
            midInterstitialShow = env->GetStaticMethodID(adsClass, "InterstitialShow", "()I");
            midRewardedVideoSetUnitId = env->GetStaticMethodID(adsClass, "RewardedVideoSetUnitId", "(Ljava/lang/String;)V");
            midRewardedVideoLoad = env->GetStaticMethodID(adsClass, "RewardedVideoLoad", "()V");
            midRewardedVideoShow = env->GetStaticMethodID(adsClass, "RewardedVideoShow", "()I");

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
            jstring url_jstring = (jstring)env->NewStringUTF(unitId.c_str());
            env->CallStaticVoidMethod(adsClass, midInterstitialSetUnitId, url_jstring);
            env->DeleteLocalRef(url_jstring);
        }

        void setRewardedVideoUnit(const char* unit) {
            JNIEnv* env = getJNIEnv();
            if (!midRewardedVideoSetUnitId)
            {
                logError("RewardedVideoSetUnitId Java method not Found");
                return;
            }
            jstring url_jstring = (jstring)env->NewStringUTF(unit);
            env->CallStaticVoidMethod(adsClass, midRewardedVideoSetUnitId, url_jstring);
            env->DeleteLocalRef(url_jstring);
            return;
        }

        void v_tryLoadInterstitial() {
            JNIEnv* env = getJNIEnv();
            if (!midInterstitialLoad) {
                logError("InterstitialLoad Java method not Found");
                return;
            }
            env->CallStaticVoidMethod(adsClass, midInterstitialLoad);
        }

        void v_tryLoadRewardedVideo() {
            JNIEnv* env = getJNIEnv();
            if (!midRewardedVideoLoad) {
                logError("RewardedVideoLoad Java method not Found");
                return;
            }
            env->CallStaticVoidMethod(adsClass, midRewardedVideoLoad);
        }

        void v_showInterstitial() {
            JNIEnv* env = getJNIEnv();
            if (!midInterstitialShow) {
                logError("InterstitialShow Java method not Found");
                return;
            }
            env->CallStaticIntMethod(adsClass, midInterstitialShow);
        }

        void v_showRewardedVideo() {
            JNIEnv* env = getJNIEnv();
            if (!midRewardedVideoShow) {
                logError("RewardedVideoShow Java method not Found");
                return;
            }
            env->CallStaticIntMethod(adsClass, midRewardedVideoShow);
        }

        JNIEnv* getJNIEnv() {
            return jniEnv;
        }
    private:
        JNIEnv* jniEnv;
        jclass adsClass;
        jmethodID midInterstitialSetUnitId;
        jmethodID midInterstitialLoad;
        jmethodID midInterstitialShow;
        jmethodID midRewardedVideoSetUnitId;
        jmethodID midRewardedVideoLoad;
        jmethodID midRewardedVideoShow;
    };
}

#endif // __AKK0RD_SDK_ADS_ANDROIDPROVIDER_H__