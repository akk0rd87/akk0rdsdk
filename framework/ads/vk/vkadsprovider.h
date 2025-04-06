#ifndef __AKK0RD_SDK_ADS_VKADS_PROVIDER_H__
#define __AKK0RD_SDK_ADS_VKADS_PROVIDER_H__

#include <vector>
#include <string>
#include "ads/adprovider.h"

namespace ads {
    namespace VKAds {
        class Provider : public ads::Provider {
        public:
            Provider(std::weak_ptr<ads::ProviderCallback> Callback) :
                ads::Provider(Callback)
            {}
        };
    };
};
#endif