#ifndef __AKK0RD_SDK_ADS_YANDEX_WINPROVIDER_H__
#define __AKK0RD_SDK_ADS_YANDEX_WINPROVIDER_H__

#include "../yandexadsprovider.h"

namespace ads {
    namespace Yandex {
        class WindowsProvider : public Provider {
        public:
            WindowsProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) : Provider(callback) {
                rewardedVideoStatus = ads::RewardedVideoStatus::ReadyToLoad;
            }

            static void setStaticProvider(std::shared_ptr<WindowsProvider>& provider) {
            }

            virtual void addInterstitialUnit(const char* unit, timeMS timePriority) {
                // do nothing instead of adding unit to vector
            }

        protected:
            // На Windows нет реального SDK для показа рекламы. Rewarded video считаем всегда загруженной,
            // а показ мгновенно засчитывает вознаграждение — чтобы было проще отлаживать подсказку за рекламу.
            virtual bool isInited() const override { return true; }
            virtual void v_tryLoadRewardedVideo() override { eventCallback(ads::Event::RewardedVideoLoaded); }
            virtual void v_showRewardedVideo() override { eventCallback(ads::Event::RewardedVideoRewarded); }
        };
    }
}
#endif