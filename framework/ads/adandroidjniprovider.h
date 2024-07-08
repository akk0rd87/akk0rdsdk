#ifndef __AKK0RD_SDK_ADS_ANDROIDPROVIDER_H__
#define __AKK0RD_SDK_ADS_ANDROIDPROVIDER_H__

#include <jni.h>
#include "adunitstatus.h"
#include "core/platformwrapper/android/android_javautf8_string.h"

#define AKKORSDK_ANDROIDJNIPROVIDER_IMPLEMENTATION(java_class_name) \
class AndroidProvider : public Provider { \
public: \
    AndroidProvider(JNIEnv* jnienv, std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) : \
        Provider(callback), \
        jniProvider(java_class_name, jnienv, format, interstitialStatus, rewardedVideoStatus) {} \
    void onAdEvent(int eventType) { \
        eventCallback(static_cast<ads::Event>(eventType)); \
    } \
    static void setStaticProvider(std::shared_ptr<AndroidProvider>& provider) { \
        staticProvider = provider; \
    } \
    static bool wasInited; \
    static std::weak_ptr<AndroidProvider> staticProvider; \
protected: \
    virtual void InterstitialSetUnitId(const std::string& unitId) override { \
        jniProvider.InterstitialSetUnitId(unitId); \
    } \
    virtual void setRewardedVideoUnit(const char* unit) override { \
        jniProvider.setRewardedVideoUnit(unit); \
    } \
    virtual void v_tryLoadInterstitial() override { \
        jniProvider.v_tryLoadInterstitial(); \
    } \
    virtual void v_tryLoadRewardedVideo() override { \
        jniProvider.v_tryLoadRewardedVideo(); \
    } \
    virtual void v_showInterstitial() override { \
        jniProvider.v_showInterstitial(); \
    } \
    virtual void v_showRewardedVideo() override { \
        jniProvider.v_showRewardedVideo(); \
    } \
    virtual bool isInited() const override { return wasInited; } \
private: \
    ads::AndroidJNIProvider jniProvider; \
}

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

            jmethodID initMethod = env->GetStaticMethodID(adsGlobalRef, "initialize", "()V");
            if (!initMethod) {
                logError("Initialize Java method not Found");
                return;
            }

            {
                const auto interstitialInit = (!(!(format & ads::Format::Interstitial))) ? 1 : 0;
                const auto rewardedvideoInit = (!(!(format & ads::Format::RewardedVideo))) ? 1 : 0;
                env->CallStaticVoidMethod(adsGlobalRef, initMethod);

                if (interstitialInit) {
                    interstitialStatus = ads::InterstitialStatus::ReadyToLoad;
                }

                if (rewardedvideoInit) {
                    rewardedVideoStatus = ads::RewardedVideoStatus::ReadyToLoad;
                }
            }

            midInterstitialSetUnitId = env->GetStaticMethodID(adsGlobalRef, "interstitialSetUnitId", "(Ljava/lang/String;)V");
            midInterstitialLoad = env->GetStaticMethodID(adsGlobalRef, "interstitialLoad", "()V");
            midInterstitialShow = env->GetStaticMethodID(adsGlobalRef, "interstitialShow", "()I");
            midRewardedVideoSetUnitId = env->GetStaticMethodID(adsGlobalRef, "rewardedVideoSetUnitId", "(Ljava/lang/String;)V");
            midRewardedVideoLoad = env->GetStaticMethodID(adsGlobalRef, "rewardedVideoLoad", "()V");
            midRewardedVideoShow = env->GetStaticMethodID(adsGlobalRef, "rewardedVideoShow", "()I");

            if (!midInterstitialSetUnitId) { logError("midInterstitialSetUnitId Java method not found"); }
            if (!midInterstitialLoad) { logError("midInterstitialLoad Java method not found"); }
            if (!midInterstitialShow) { logError("midInterstitialShow Java method not found"); }
            if (!midRewardedVideoSetUnitId) { logError("midRewardedVideoSetUnitId Java method not found"); }
            if (!midRewardedVideoLoad) { logError("midRewardedVideoLoad Java method not found"); }
            if (!midRewardedVideoShow) { logError("midRewardedVideoShow Java method not found"); }
        }

        void InterstitialSetUnitId(const std::string& unitId) {
            if (!midInterstitialSetUnitId) {
                logError("InterstitialSetUnitId Java method not Found");
                return;
            }
            JNIEnv* env = getJNIEnv();
            AndroidJavaUTF8String url_jstring(env, unitId.c_str());
            env->CallStaticVoidMethod(adsGlobalRef, midInterstitialSetUnitId, url_jstring.get());
        }

        void setRewardedVideoUnit(const char* unit) {
            if (!midRewardedVideoSetUnitId)
            {
                logError("rewardedVideoSetUnitId Java method not Found");
                return;
            }
            JNIEnv* env = getJNIEnv();
            AndroidJavaUTF8String url_jstring(env, unit);
            env->CallStaticVoidMethod(adsGlobalRef, midRewardedVideoSetUnitId, url_jstring.get());
            return;
        }

        void v_tryLoadInterstitial() {
            if (!midInterstitialLoad) {
                logError("interstitialLoad Java method not Found");
                return;
            }
            JNIEnv* env = getJNIEnv();
            env->CallStaticVoidMethod(adsGlobalRef, midInterstitialLoad);
        }

        void v_tryLoadRewardedVideo() {
            if (!midRewardedVideoLoad) {
                logError("rewardedVideoLoad Java method not Found");
                return;
            }
            JNIEnv* env = getJNIEnv();
            env->CallStaticVoidMethod(adsGlobalRef, midRewardedVideoLoad);
        }

        void v_showInterstitial() {
            if (!midInterstitialShow) {
                logError("interstitialShow Java method not Found");
                return;
            }
            JNIEnv* env = getJNIEnv();
            env->CallStaticIntMethod(adsGlobalRef, midInterstitialShow);
        }

        void v_showRewardedVideo() {
            if (!midRewardedVideoShow) {
                logError("rewardedVideoShow Java method not Found");
                return;
            }
            JNIEnv* env = getJNIEnv();
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