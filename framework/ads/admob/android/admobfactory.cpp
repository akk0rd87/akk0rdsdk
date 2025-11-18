#include "../admobfactory.h"
#include "basewrapper.h"
#include "impladmobandroid.h"
#include "core/platformwrapper/android/android_wrapper.h"

std::shared_ptr<ads::AdMob::Provider> ads::AdMob::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::AdMob::AndroidProvider>(AndroidWrapper::GetJNIEnv(), callback, format);
    ads::AdMob::AndroidProvider::setStaticProvider(provider);
    return provider;
};
