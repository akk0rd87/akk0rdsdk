#include "../yandexadsfactory.h"
#include "implyandexadsandroid.h"

std::shared_ptr<ads::Yandex::Provider> ads::Yandex::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::Yandex::AndroidProvider>(callback, format);
    ads::Yandex::AndroidProvider::setStaticProvider(provider);
    return provider;
};