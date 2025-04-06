#ifndef __AKK0RD_SDK_ADS_VKADS_ANDROIDPROVIDER_H__
#define __AKK0RD_SDK_ADS_VKADS_ANDROIDPROVIDER_H__

#include <jni.h>
#include "../vkadsprovider.h"
#include "../../adandroidjniprovider.h"

namespace ads {
    namespace VKAds {
        AKKORSDK_ANDROIDJNIPROVIDER_IMPLEMENTATION("org/akkord/lib/VKAdsAdapter");
    };
}

using providerName = ads::VKAds::AndroidProvider;

bool providerName::wasInited = false;
std::weak_ptr<providerName> providerName::staticProvider;

extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_VKAdsAdapterKt_adCallback(JNIEnv*, jclass, jint EventType) {
        if (auto cbk = providerName::staticProvider.lock()) {
            cbk->onAdEvent(static_cast<int>(EventType));
        }
    }

    JNIEXPORT void JNICALL Java_org_akkord_lib_VKAdsAdapterKt_initCallback(JNIEnv*, jclass, jint code) {
        // if zero code means success
        providerName::wasInited = (static_cast<int>(code) ? false : true);
    }
}

#endif