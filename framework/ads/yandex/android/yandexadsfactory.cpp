#include "../yandexadsfactory.h"
#include "basewrapper.h"
#include "implyandexadsandroid.h"
#include "core/platformwrapper/android/android_wrapper.h"

std::shared_ptr<ads::Yandex::Provider> ads::Yandex::createProvider(std::weak_ptr<ads::ProviderCallback> callback, ads::Format format) {
    auto provider = std::make_shared<ads::Yandex::AndroidProvider>(AndroidWrapper::GetJNIEnv(), callback, format);
    ads::Yandex::AndroidProvider::setStaticProvider(provider);
    return provider;
};