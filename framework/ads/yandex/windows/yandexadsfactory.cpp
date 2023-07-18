#include "../yandexadsfactory.h"
#include "implyandexadswindows.h"

std::shared_ptr<ads::Yandex::Provider> ads::Yandex::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::Yandex::WindowsProvider>(callback, format);
    ads::Yandex::WindowsProvider::setStaticProvider(provider);
    return provider;
};