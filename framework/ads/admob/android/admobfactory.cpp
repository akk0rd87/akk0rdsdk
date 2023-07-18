#include "../admobfactory.h"
#include "impladmobandroid.h"

std::shared_ptr<ads::AdMob::Provider> ads::AdMob::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::AdMob::AndroidProvider>(callback, format);
    ads::AdMob::AndroidProvider::setStaticProvider(provider);
    return provider;
};
