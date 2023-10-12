#ifndef __AKK0RD_SDK_ADS_ADMOB_ANDROIDPROVIDER_H__
#define __AKK0RD_SDK_ADS_ADMOB_ANDROIDPROVIDER_H__

#include <jni.h>
#include "../admobprovider.h"
#include "../../adandroidjniprovider.h"

namespace ads {
    namespace AdMob {
        class AndroidProvider : public Provider {
        public:
            AndroidProvider(JNIEnv* jnienv, std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) :
                Provider(callback),
                jniProvider("org/akkord/lib/AdMobAdapter", jnienv, format, interstitialStatus, rewardedVideoStatus) {}

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
                jniProvider.InterstitialSetUnitId(unitId);
            }

            virtual void setRewardedVideoUnit(const char* unit) override {
                jniProvider.setRewardedVideoUnit(unit);
            }

            virtual void v_tryLoadInterstitial() override {
                jniProvider.v_tryLoadInterstitial();
            }

            virtual void v_tryLoadRewardedVideo() override {
                jniProvider.v_tryLoadRewardedVideo();
            }

            virtual void v_showInterstitial() override {
                jniProvider.v_showInterstitial();
            }

            virtual void v_showRewardedVideo() override {
                jniProvider.v_showRewardedVideo();
            }
            virtual bool isInited() const override { return wasInited; }

        private:
            ads::AndroidJNIProvider jniProvider;
        };
    };
};

using providerName = ads::AdMob::AndroidProvider;

bool providerName::wasInited = false;
std::weak_ptr<providerName> providerName::staticProvider;

extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_AdMobAdapter_AdCallback(JNIEnv*, jclass, jint EventType) {
        if (auto cbk = providerName::staticProvider.lock()) {
            cbk->onAdEvent(static_cast<int>(EventType));
        }
    }

    JNIEXPORT void JNICALL Java_org_akkord_lib_AdMobAdapter_InitCallback(JNIEnv*, jclass, jint code) {
        // if zero code means success
        providerName::wasInited = (static_cast<int>(code) ? false : true);
    }
}

#endif