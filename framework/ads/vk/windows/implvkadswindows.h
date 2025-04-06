#ifndef __AKK0RD_SDK_ADS_VKADS_WINDOWS_PROVIDER_H__
#define __AKK0RD_SDK_ADS_VKADS_WINDOWS_PROVIDER_H__

#include "../vkadsprovider.h"

namespace ads {
    namespace VKAds {
        class WindowsProvider : public Provider {
        public:
            WindowsProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) : Provider(callback) {}

            static void setStaticProvider(std::shared_ptr<WindowsProvider>& provider) {
            }

            virtual void addInterstitialUnit(const char* unit, timeMS timePriority) {
                // do nothing instead of adding unit to vector
            }
        };
    };
};
#endif