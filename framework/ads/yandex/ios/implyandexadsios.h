#ifndef __AKK0RD_SDK_ADS_ADMOB_IOSPROVIDER_H__
#define __AKK0RD_SDK_ADS_ADMOB_IOSPROVIDER_H__

#include "../yandexadsprovider.h"

namespace ads {
    namespace Yandex {
        class iOSProvider : public Provider {
        public:
            iOSProvider(std::weak_ptr<ads::ProviderCallback> cbk, ads::Format format);

            static void onAdEvent(ads::Event event) {
                if (auto cbk = staticProvider.lock()) {
                    cbk->eventCallback(event);
                }
            }

            static void setStaticProvider(std::shared_ptr<iOSProvider>& provider) {
                staticProvider = provider;
            }

            static std::weak_ptr<iOSProvider> staticProvider;
            static bool wasInited;

        protected:
            virtual void InterstitialSetUnitId(const std::string& unitId) override;
            virtual void setRewardedVideoUnit(const char* unit) override;
            virtual void v_tryLoadInterstitial() override;
            virtual void v_tryLoadRewardedVideo() override;
            virtual void v_showInterstitial() override;
            virtual void v_showRewardedVideo() override;
            virtual bool isInited() const override { return wasInited; };
        };
    };
};

#endif
