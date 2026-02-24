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
            javaVM(nullptr),
            adsGlobalRef(nullptr),
            midInterstitialSetUnitId(nullptr),
            midInterstitialLoad(nullptr),
            midInterstitialShow(nullptr),
            midRewardedVideoSetUnitId(nullptr),
            midRewardedVideoLoad(nullptr),
            midRewardedVideoShow(nullptr)
        {
            if (!jnienv) {
                logError("JNIEnv is null");
                return;
            }
            jnienv->GetJavaVM(&javaVM);
            JNIEnv* env = jnienv;

            {
                jclass adsClass = env->FindClass(javaClass);
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                    logError("Exception during FindClass");
                    return;
                }
                if (!adsClass) {
                    logError("FindClass returned null");
                    return;
                }
                adsGlobalRef = (jclass)env->NewGlobalRef(adsClass);
                env->DeleteLocalRef(adsClass);
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                    logError("Exception during NewGlobalRef");
                    return;
                }
            }

            if (!adsGlobalRef) {
                logError("Could not find class");
                return;
            }

            jmethodID initMethod = env->GetStaticMethodID(adsGlobalRef, "initialize", "()V");
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during GetStaticMethodID for initialize");
                return;
            }
            if (!initMethod) {
                logError("Initialize Java method not Found");
                return;
            }

            {
                const auto interstitialInit = (!(!(format & ads::Format::Interstitial))) ? 1 : 0;
                const auto rewardedvideoInit = (!(!(format & ads::Format::RewardedVideo))) ? 1 : 0;
                env->CallStaticVoidMethod(adsGlobalRef, initMethod);
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                    logError("Exception during initialize call");
                    return;
                }

                if (interstitialInit) {
                    interstitialStatus = ads::InterstitialStatus::ReadyToLoad;
                }

                if (rewardedvideoInit) {
                    rewardedVideoStatus = ads::RewardedVideoStatus::ReadyToLoad;
                }
            }

            midInterstitialSetUnitId = env->GetStaticMethodID(adsGlobalRef, "interstitialSetUnitId", "(Ljava/lang/String;)V");
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during GetStaticMethodID for interstitialSetUnitId");
            }
            midInterstitialLoad = env->GetStaticMethodID(adsGlobalRef, "interstitialLoad", "()V");
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during GetStaticMethodID for interstitialLoad");
            }
            midInterstitialShow = env->GetStaticMethodID(adsGlobalRef, "interstitialShow", "()I");
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during GetStaticMethodID for interstitialShow");
            }
            midRewardedVideoSetUnitId = env->GetStaticMethodID(adsGlobalRef, "rewardedVideoSetUnitId", "(Ljava/lang/String;)V");
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during GetStaticMethodID for rewardedVideoSetUnitId");
            }
            midRewardedVideoLoad = env->GetStaticMethodID(adsGlobalRef, "rewardedVideoLoad", "()V");
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during GetStaticMethodID for rewardedVideoLoad");
            }
            midRewardedVideoShow = env->GetStaticMethodID(adsGlobalRef, "rewardedVideoShow", "()I");
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during GetStaticMethodID for rewardedVideoShow");
            }

            if (!midInterstitialSetUnitId) { logError("midInterstitialSetUnitId Java method not found"); }
            if (!midInterstitialLoad) { logError("midInterstitialLoad Java method not found"); }
            if (!midInterstitialShow) { logError("midInterstitialShow Java method not found"); }
            if (!midRewardedVideoSetUnitId) { logError("midRewardedVideoSetUnitId Java method not found"); }
            if (!midRewardedVideoLoad) { logError("midRewardedVideoLoad Java method not found"); }
            if (!midRewardedVideoShow) { logError("midRewardedVideoShow Java method not found"); }
        }

        ~AndroidJNIProvider() {
            if (adsGlobalRef) {
                auto scopedEnv = getJNIEnv();
                JNIEnv* env = scopedEnv.get();
                if (env) {
                    env->DeleteGlobalRef(adsGlobalRef);
                    adsGlobalRef = nullptr;
                }
            }
        }

        void InterstitialSetUnitId(const std::string& unitId) {
            if (!adsGlobalRef) {
                logError("adsGlobalRef is null");
                return;
            }
            if (!midInterstitialSetUnitId) {
                logError("InterstitialSetUnitId Java method not Found");
                return;
            }
            auto scopedEnv = getJNIEnv();
            JNIEnv* env = scopedEnv.get();
            if (!env) {
                logError("Failed to get JNIEnv");
                return;
            }

            AndroidJavaUTF8String url_jstring(env, unitId.c_str());
            env->CallStaticVoidMethod(adsGlobalRef, midInterstitialSetUnitId, url_jstring.get());
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during interstitialSetUnitId call");
            }
        }

        void setRewardedVideoUnit(const char* unit) {
            if (!adsGlobalRef) {
                logError("adsGlobalRef is null");
                return;
            }
            if (!midRewardedVideoSetUnitId)
            {
                logError("rewardedVideoSetUnitId Java method not Found");
                return;
            }
            auto scopedEnv = getJNIEnv();
            JNIEnv* env = scopedEnv.get();
            if (!env) {
                logError("Failed to get JNIEnv");
                return;
            }

            AndroidJavaUTF8String url_jstring(env, unit);
            env->CallStaticVoidMethod(adsGlobalRef, midRewardedVideoSetUnitId, url_jstring.get());
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during rewardedVideoSetUnitId call");
            }
            return;
        }

        void v_tryLoadInterstitial() {
            if (!adsGlobalRef) {
                logError("adsGlobalRef is null");
                return;
            }
            if (!midInterstitialLoad) {
                logError("interstitialLoad Java method not Found");
                return;
            }
            auto scopedEnv = getJNIEnv();
            JNIEnv* env = scopedEnv.get();
            if (!env) {
                logError("Failed to get JNIEnv");
                return;
            }

            env->CallStaticVoidMethod(adsGlobalRef, midInterstitialLoad);
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during interstitialLoad call");
            }
        }

        void v_tryLoadRewardedVideo() {
            if (!adsGlobalRef) {
                logError("adsGlobalRef is null");
                return;
            }
            if (!midRewardedVideoLoad) {
                logError("rewardedVideoLoad Java method not Found");
                return;
            }
            auto scopedEnv = getJNIEnv();
            JNIEnv* env = scopedEnv.get();
            if (!env) {
                logError("Failed to get JNIEnv");
                return;
            }

            env->CallStaticVoidMethod(adsGlobalRef, midRewardedVideoLoad);
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during rewardedVideoLoad call");
            }
        }

        void v_showInterstitial() {
            if (!adsGlobalRef) {
                logError("adsGlobalRef is null");
                return;
            }
            if (!midInterstitialShow) {
                logError("interstitialShow Java method not Found");
                return;
            }
            auto scopedEnv = getJNIEnv();
            JNIEnv* env = scopedEnv.get();
            if (!env) {
                logError("Failed to get JNIEnv");
                return;
            }

            env->CallStaticIntMethod(adsGlobalRef, midInterstitialShow);
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during interstitialShow call");
            }
        }

        void v_showRewardedVideo() {
            if (!adsGlobalRef) {
                logError("adsGlobalRef is null");
                return;
            }
            if (!midRewardedVideoShow) {
                logError("rewardedVideoShow Java method not Found");
                return;
            }
            auto scopedEnv = getJNIEnv();
            JNIEnv* env = scopedEnv.get();
            if (!env) {
                logError("Failed to get JNIEnv");
                return;
            }

            env->CallStaticIntMethod(adsGlobalRef, midRewardedVideoShow);
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                logError("Exception during rewardedVideoShow call");
            }
        }

        struct ScopedJNIEnv {
            JavaVM* vm;
            JNIEnv* env;
            bool needsDetach;

            explicit ScopedJNIEnv(JavaVM* javaVM) : vm(javaVM), env(nullptr), needsDetach(false) {
                if (!vm) return;
                jint status = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
                if (status == JNI_EDETACHED) {
                    if (vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
                        needsDetach = true;
                    } else {
                        env = nullptr;
                    }
                } else if (status != JNI_OK) {
                    env = nullptr;
                }
            }

            ~ScopedJNIEnv() {
                if (needsDetach && vm) {
                    vm->DetachCurrentThread();
                }
            }

            ScopedJNIEnv(const ScopedJNIEnv&) = delete;
            ScopedJNIEnv& operator=(const ScopedJNIEnv&) = delete;

            ScopedJNIEnv(ScopedJNIEnv&& other) noexcept
                : vm(other.vm), env(other.env), needsDetach(other.needsDetach) {
                other.needsDetach = false;
                other.env = nullptr;
            }
            ScopedJNIEnv& operator=(ScopedJNIEnv&&) = delete;

            JNIEnv* get() const { return env; }
        };

        ScopedJNIEnv getJNIEnv() {
            return ScopedJNIEnv(javaVM);
        }
    private:
        JavaVM* javaVM;
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