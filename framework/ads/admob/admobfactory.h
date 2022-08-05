#ifndef __AKK0RD_SDK_ADS_ADMOB_FACTORY_H__
#define __AKK0RD_SDK_ADS_ADMOB_FACTORY_H__

#include <memory>
#include "admobprovider.h"

namespace ads {
    namespace AdMob {
        std::shared_ptr<ads::AdMob::Provider> createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format);
    };
};

#endif