#include "basewrapper.h"
#include "admobfactory.h"

#ifdef __WINDOWS__
#include "impladmobwindows.h"
using ProviderImpl = ads::AdMob::WindowsProvider;
#endif

#ifdef __ANDROID__
#include "impladmobandroid.h"
using ProviderImpl = ads::AdMob::AndroidProvider;
#endif

#ifdef __APPLE__
#include "impladmobios.h"
using ProviderImpl = ads::AdMob::iOSProvider;
#endif

std::shared_ptr<ads::AdMob::Provider> ads::AdMob::createAdmobProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ProviderImpl>(callback, format);
    ProviderImpl::setStaticProvider(provider);
    return provider;
};
