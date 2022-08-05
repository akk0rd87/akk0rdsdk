#include "basewrapper.h"
#include "yandexadsfactory.h"

#ifdef __WINDOWS__
#include "windows/implyandexadswindows.h"
using ProviderImpl = ads::Yandex::WindowsProvider;
#endif

#ifdef __ANDROID__
#include "android/implyandexadsandroid.h"
using ProviderImpl = ads::Yandex::AndroidProvider;
#endif

#ifdef __APPLE__
#include "ios/implyandexadsios.h"
using ProviderImpl = ads::Yandex::iOSProvider;
#endif

std::shared_ptr<ads::Yandex::Provider> ads::Yandex::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ProviderImpl>(callback, format);
    ProviderImpl::setStaticProvider(provider);
    return provider;
};