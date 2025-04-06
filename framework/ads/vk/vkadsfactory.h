#ifndef __AKK0RD_SDK_ADS_ADADS_FACTORY_H__
#define __AKK0RD_SDK_ADS_ADADS_FACTORY_H__

#include <memory>
#include "vkadsprovider.h"

namespace ads {
    namespace VKAds {
        std::shared_ptr<ads::VKAds::Provider> createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format);
    };
};

#endif