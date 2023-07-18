#include "../admobfactory.h"
#include "impladmobwindows.h"

std::shared_ptr<ads::AdMob::Provider> ads::AdMob::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::AdMob::WindowsProvider>(callback, format);
    ads::AdMob::WindowsProvider::setStaticProvider(provider);
    return provider;
};
