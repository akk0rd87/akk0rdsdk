#ifndef __AKK0RD_SDK_ADS_ADMOB_ANDROIDPROVIDER_H__
#define __AKK0RD_SDK_ADS_ADMOB_ANDROIDPROVIDER_H__

#include <jni.h>
#include "admobprovider.h"

namespace ads {
    namespace AdMob {
        class AndroidProvider : public Provider {
        public:
            AndroidProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) :
            Provider                  (callback),
            AdMobClass                (nullptr),
            midInterstitialSetUnitId  (nullptr),
            midInterstitialLoad       (nullptr),
            midInterstitialShow       (nullptr),
            midRewardedVideoSetUnitId (nullptr),
            midRewardedVideoLoad      (nullptr),
            midRewardedVideoShow      (nullptr)
            {
                wasInited = false;
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                jclass localClass = env->FindClass("org/akkord/lib/AdMobAdapter");

                if(!localClass) {
                    logError("Could not find AdMobAdapter class");
                    return;
                }

                AdMobClass = reinterpret_cast<jclass>(env->NewGlobalRef(localClass));
                env->DeleteLocalRef(localClass);

                jmethodID AdmobInit = env->GetStaticMethodID(AdMobClass, "Initialize", "()V");
                if(!AdmobInit) {
                    logError("Initialize Java method not Found");
                    return;
                }

                env->CallStaticVoidMethod(AdMobClass, AdmobInit);

                if(!(!(format & ads::Format::Interstitial))) {
                    // InterstitialInit method
                    AdmobInit = env->GetStaticMethodID(AdMobClass, "InterstitialInit", "()V");
                    if(!AdmobInit) {
                        logError("InterstitialInit Java method not Found");
                        return;
                    }
                    env->CallStaticVoidMethod(AdMobClass, AdmobInit);
                    interstitialStatus = ads::Provider::InterstitialStatus::ReadyToLoad;
                }

                if(!(!(format & ads::Format::RewardedVideo))) {
                    // RewardedVideoInit method
                    AdmobInit = env->GetStaticMethodID(AdMobClass, "RewardedVideoInit", "()V");
                    if(!AdmobInit) {
                        logError("RewardedVideoInit Java method not Found");
                        return;
                    }
                    env->CallStaticVoidMethod(AdMobClass, AdmobInit);
                    rewardedVideoStatus = ads::Provider::RewardedVideoStatus::ReadyToLoad;
                }

                midInterstitialSetUnitId   = env->GetStaticMethodID(AdMobClass, "InterstitialSetUnitId", "(Ljava/lang/String;)V");
                midInterstitialLoad        = env->GetStaticMethodID(AdMobClass, "InterstitialLoad", "()V");
                midInterstitialShow        = env->GetStaticMethodID(AdMobClass, "InterstitialShow", "()I");
                midRewardedVideoSetUnitId  = env->GetStaticMethodID(AdMobClass, "RewardedVideoSetUnitId", "(Ljava/lang/String;)V");
                midRewardedVideoLoad       = env->GetStaticMethodID(AdMobClass, "RewardedVideoLoad", "()V");
                midRewardedVideoShow       = env->GetStaticMethodID(AdMobClass, "RewardedVideoShow", "()I");

                if(!midInterstitialSetUnitId ) { logError("midInterstitialSetUnitId  Java method not found"); }
                if(!midInterstitialLoad      ) { logError("midInterstitialLoad       Java method not found"); }
                if(!midInterstitialShow      ) { logError("midInterstitialShow       Java method not found"); }
                if(!midRewardedVideoSetUnitId) { logError("midRewardedVideoSetUnitId Java method not found"); }
                if(!midRewardedVideoLoad     ) { logError("midRewardedVideoLoad      Java method not found"); }
                if(!midRewardedVideoShow     ) { logError("midRewardedVideoShow      Java method not found"); }
            }

            void onAdEvent(int adType, int eventType, int code) {
                eventCallback(eventType);
            }

            static void setStaticProvider(std::shared_ptr<AndroidProvider>& provider) {
                staticProvider = provider;
            }

            static bool wasInited;
            static std::weak_ptr<AndroidProvider> staticProvider;

        protected:
            virtual void InterstitialSetUnitId(const std::string& unitId) override {
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                if(!midInterstitialSetUnitId) {
                    logError("InterstitialSetUnitId Java method not Found");
                    return;
                }
                jstring url_jstring = (jstring)env->NewStringUTF(unitId.c_str());
                env->CallStaticVoidMethod(AdMobClass, midInterstitialSetUnitId, url_jstring);
                env->DeleteLocalRef(url_jstring);
            }

            virtual void setRewardedVideoUnit(const char* unit) override {
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                if(!midRewardedVideoSetUnitId)
                {
                    logError("RewardedVideoSetUnitId Java method not Found");
                    return;
                }
                jstring url_jstring = (jstring)env->NewStringUTF(unit);
                env->CallStaticVoidMethod(AdMobClass, midRewardedVideoSetUnitId, url_jstring);
                env->DeleteLocalRef(url_jstring);
                return;
            }

            virtual void admob_tryLoadInterstitial() override {
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                if(!midInterstitialLoad) {
                    logError("InterstitialLoad Java method not Found");
                    return;
                }
                env->CallStaticVoidMethod(AdMobClass, midInterstitialLoad);
            }

            virtual void admob_tryLoadRewardedVideo() override {
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                if(!midRewardedVideoLoad) {
                    logError("RewardedVideoLoad Java method not Found");
                    return;
                }
                env->CallStaticVoidMethod(AdMobClass, midRewardedVideoLoad);
            }

            virtual void v_showInterstitial() override {
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                if(!midInterstitialShow) {
                    logError("InterstitialShow Java method not Found");
                    return;
                }
                env->CallStaticIntMethod(AdMobClass, midInterstitialShow);
            }

            virtual void v_showRewardedVideo() override {
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                if(!midRewardedVideoShow) {
                    logError("RewardedVideoShow Java method not Found");
                    return;
                }
                env->CallStaticIntMethod(AdMobClass, midRewardedVideoShow);
            }

            virtual bool isInited() const override { return wasInited; }

            jclass AdMobClass;
            jmethodID midInterstitialSetUnitId;
            jmethodID midInterstitialLoad;
            jmethodID midInterstitialShow;
            jmethodID midRewardedVideoSetUnitId;
            jmethodID midRewardedVideoLoad;
            jmethodID midRewardedVideoShow;
        };
    };
};

bool                                       ads::AdMob::AndroidProvider::wasInited = false;
std::weak_ptr<ads::AdMob::AndroidProvider> ads::AdMob::AndroidProvider::staticProvider;

extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_AdMobAdapter_AdCallback(JNIEnv*, jclass, jint, jint, jint);
    JNIEXPORT void JNICALL Java_org_akkord_lib_AdMobAdapter_InitCallback(JNIEnv*, jclass, jint);
}
JNIEXPORT void JNICALL Java_org_akkord_lib_AdMobAdapter_AdCallback(JNIEnv*, jclass, jint AdType, jint EventType, jint Code) {
    if(auto cbk = ads::AdMob::AndroidProvider::staticProvider.lock()) {
        cbk->onAdEvent(static_cast<int>(AdType), static_cast<int>(EventType), static_cast<int>(Code));
    }
}

JNIEXPORT void JNICALL Java_org_akkord_lib_AdMobAdapter_InitCallback(JNIEnv*, jclass, jint code) {
    // if zero code means success
    ads::AdMob::AndroidProvider::wasInited = (static_cast<int>(code) ? false : true);
}
#endif