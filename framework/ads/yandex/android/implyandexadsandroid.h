#ifndef __AKK0RD_SDK_ADS_ADMOB_ANDROIDPROVIDER_H__
#define __AKK0RD_SDK_ADS_ADMOB_ANDROIDPROVIDER_H__

#include <jni.h>
#include "../yandexadsprovider.h"

namespace ads {
    namespace Yandex {
        class AndroidProvider : public Provider {
        public:
            AndroidProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) :
            Provider                  (callback),
            YandexAdsClass            (nullptr),
            midInterstitialSetUnitId  (nullptr),
            midInterstitialLoad       (nullptr),
            midInterstitialShow       (nullptr),
            midRewardedVideoSetUnitId (nullptr),
            midRewardedVideoLoad      (nullptr),
            midRewardedVideoShow      (nullptr)
            {
                wasInited = false;
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                jclass localClass = env->FindClass("org/akkord/lib/YandexAdsAdapter");

                if(!localClass) {
                    logError("Could not find YandexAdsAdapter class");
                    return;
                }

                YandexAdsClass = reinterpret_cast<jclass>(env->NewGlobalRef(localClass));
                env->DeleteLocalRef(localClass);

                jmethodID YandexInit = env->GetStaticMethodID(YandexAdsClass, "Initialize", "(II)V");
                if(!YandexInit) {
                    logError("Initialize Java method not Found");
                    return;
                }

                {
                    const auto interstitialInit  = (!(!(format & ads::Format::Interstitial)))  ? 1 : 0;
                    const auto rewardedvideoInit = (!(!(format & ads::Format::RewardedVideo))) ? 1 : 0;
                    env->CallStaticVoidMethod(YandexAdsClass, YandexInit, interstitialInit, rewardedvideoInit);

                    if(interstitialInit) {
                        interstitialStatus = ads::Provider::InterstitialStatus::ReadyToLoad;
                    }

                    if(rewardedvideoInit) {
                        rewardedVideoStatus = ads::Provider::RewardedVideoStatus::ReadyToLoad;
                    }
                }

                midInterstitialSetUnitId   = env->GetStaticMethodID(YandexAdsClass, "InterstitialSetUnitId", "(Ljava/lang/String;)V");
                midInterstitialLoad        = env->GetStaticMethodID(YandexAdsClass, "InterstitialLoad", "()V");
                midInterstitialShow        = env->GetStaticMethodID(YandexAdsClass, "InterstitialShow", "()I");
                midRewardedVideoSetUnitId  = env->GetStaticMethodID(YandexAdsClass, "RewardedVideoSetUnitId", "(Ljava/lang/String;)V");
                midRewardedVideoLoad       = env->GetStaticMethodID(YandexAdsClass, "RewardedVideoLoad", "()V");
                midRewardedVideoShow       = env->GetStaticMethodID(YandexAdsClass, "RewardedVideoShow", "()I");

                if(!midInterstitialSetUnitId ) { logError("midInterstitialSetUnitId  Java method not found"); }
                if(!midInterstitialLoad      ) { logError("midInterstitialLoad       Java method not found"); }
                if(!midInterstitialShow      ) { logError("midInterstitialShow       Java method not found"); }
                if(!midRewardedVideoSetUnitId) { logError("midRewardedVideoSetUnitId Java method not found"); }
                if(!midRewardedVideoLoad     ) { logError("midRewardedVideoLoad      Java method not found"); }
                if(!midRewardedVideoShow     ) { logError("midRewardedVideoShow      Java method not found"); }
            }

            void onAdEvent(int eventType) {
                eventCallback(static_cast<ads::Event>(eventType));
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
                env->CallStaticVoidMethod(YandexAdsClass, midInterstitialSetUnitId, url_jstring);
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
                env->CallStaticVoidMethod(YandexAdsClass, midRewardedVideoSetUnitId, url_jstring);
                env->DeleteLocalRef(url_jstring);
                return;
            }

            virtual void v_tryLoadInterstitial() override {
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                if(!midInterstitialLoad) {
                    logError("InterstitialLoad Java method not Found");
                    return;
                }
                env->CallStaticVoidMethod(YandexAdsClass, midInterstitialLoad);
            }

            virtual void v_tryLoadRewardedVideo() override {
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                if(!midRewardedVideoLoad) {
                    logError("RewardedVideoLoad Java method not Found");
                    return;
                }
                env->CallStaticVoidMethod(YandexAdsClass, midRewardedVideoLoad);
            }

            virtual void v_showInterstitial() override {
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                if(!midInterstitialShow) {
                    logError("InterstitialShow Java method not Found");
                    return;
                }
                env->CallStaticIntMethod(YandexAdsClass, midInterstitialShow);
            }

            virtual void v_showRewardedVideo() override {
                JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
                if(!midRewardedVideoShow) {
                    logError("RewardedVideoShow Java method not Found");
                    return;
                }
                env->CallStaticIntMethod(YandexAdsClass, midRewardedVideoShow);
            }

            virtual bool isInited() const override { return wasInited; }

            jclass YandexAdsClass;
            jmethodID midInterstitialSetUnitId;
            jmethodID midInterstitialLoad;
            jmethodID midInterstitialShow;
            jmethodID midRewardedVideoSetUnitId;
            jmethodID midRewardedVideoLoad;
            jmethodID midRewardedVideoShow;
        };
    };
};

bool                                        ads::Yandex::AndroidProvider::wasInited = false;
std::weak_ptr<ads::Yandex::AndroidProvider> ads::Yandex::AndroidProvider::staticProvider;

extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_YandexAdsAdapter_AdCallback(JNIEnv*, jclass, jint);
    JNIEXPORT void JNICALL Java_org_akkord_lib_YandexAdsAdapter_InitCallback(JNIEnv*, jclass, jint);
}
JNIEXPORT void JNICALL Java_org_akkord_lib_YandexAdsAdapter_AdCallback(JNIEnv*, jclass, jint EventType) {
    if(auto cbk = ads::Yandex::AndroidProvider::staticProvider.lock()) {
        cbk->onAdEvent(static_cast<int>(EventType));
    }
}

JNIEXPORT void JNICALL Java_org_akkord_lib_YandexAdsAdapter_InitCallback(JNIEnv*, jclass, jint code) {
    // if zero code means success
    ads::Yandex::AndroidProvider::wasInited = (static_cast<int>(code) ? false : true);
}
#endif