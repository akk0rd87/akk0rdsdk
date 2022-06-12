#ifndef __AKK0RD_SDK_ADS_ADMOB_WINPROVIDER_H__
#define __AKK0RD_SDK_ADS_ADMOB_WINPROVIDER_H__

#include "admobprovider.h"

namespace ads {
    namespace AdMob {
        class WindowsProvider : public Provider {
        public:
            WindowsProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) : Provider(callback) {}

            static void setStaticProvider(std::shared_ptr<WindowsProvider>& provider) {
            }

            virtual void addInterstitialUnit(const char* unit, timeMS timePriority) {
                // do nothing instead of adding unit to vector
            }
        };
    }
}
#endif