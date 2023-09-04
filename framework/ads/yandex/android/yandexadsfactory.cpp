#include "../yandexadsfactory.h"
#include "basewrapper.h"
#include "implyandexadsandroid.h"

std::shared_ptr<ads::Yandex::Provider> ads::Yandex::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::Yandex::AndroidProvider>(static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv()), static_cast<jobject>(SDL_AndroidGetActivity()), callback, format);
    ads::Yandex::AndroidProvider::setStaticProvider(provider);
    return provider;
};