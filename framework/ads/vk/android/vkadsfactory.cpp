#include "../vkadsfactory.h"
#include "basewrapper.h"
#include "implvkadsandroid.h"

std::shared_ptr<ads::VKAds::Provider> ads::VKAds::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::VKAds::AndroidProvider>(static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv()), callback, format);
    ads::VKAds::AndroidProvider::setStaticProvider(provider);
    return provider;
};
