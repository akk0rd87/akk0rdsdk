#ifndef __AKK0RD_SDK_ADS_YANDEX_FACTORY_H__
#define __AKK0RD_SDK_ADS_YANDEX_FACTORY_H__

#include <memory>
#include "yandexadsprovider.h"

namespace ads {
    namespace Yandex {
        std::shared_ptr<ads::Yandex::Provider> createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format);
    };
};

#endif