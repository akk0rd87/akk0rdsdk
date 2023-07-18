#include "../admobfactory.h"
#include "impladmobios.h"

std::shared_ptr<ads::AdMob::Provider> ads::AdMob::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::AdMob::iOSProvider>(callback, format);
    ads::AdMob::iOSProvider::setStaticProvider(provider);
    return provider;
};
