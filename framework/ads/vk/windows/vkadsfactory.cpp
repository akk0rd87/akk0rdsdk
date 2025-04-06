#include "../vkadsfactory.h"
#include "implvkadswindows.h"

std::shared_ptr<ads::VKAds::Provider> ads::VKAds::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::VKAds::WindowsProvider>(callback, format);
    ads::VKAds::WindowsProvider::setStaticProvider(provider);
    return provider;
};
