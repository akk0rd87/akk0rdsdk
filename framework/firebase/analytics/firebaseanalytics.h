#ifndef __AKK0RD_SDK_FIREBASE_ANALYTICS__
#define __AKK0RD_SDK_FIREBASE_ANALYTICS__

#ifdef __APPLE__
#include "ios/iosfirebaseanalytics.h"
#endif

namespace Firebase {
    namespace Analytics {
        void Init() {
#ifdef __APPLE__
Firebase::Analytics::iOS::Init();
#endif
        }
    }
}

#endif