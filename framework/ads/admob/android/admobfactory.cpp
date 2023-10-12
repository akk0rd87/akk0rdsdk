#include "../admobfactory.h"
#include "basewrapper.h"
#include "impladmobandroid.h"

std::shared_ptr<ads::AdMob::Provider> ads::AdMob::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::AdMob::AndroidProvider>(static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv()), callback, format);
    ads::AdMob::AndroidProvider::setStaticProvider(provider);
    return provider;
};
