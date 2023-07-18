#include "../yandexadsfactory.h"
#include "implyandexadsios.h"

std::shared_ptr<ads::Yandex::Provider> ads::Yandex::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::Yandex::iOSProvider>(callback, format);
    ads::Yandex::iOSProvider::setStaticProvider(provider);
    return provider;
};