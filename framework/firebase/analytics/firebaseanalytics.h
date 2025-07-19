#ifndef __AKK0RD_SDK_FIREBASE_ANALYTICS__
#define __AKK0RD_SDK_FIREBASE_ANALYTICS__

#ifdef SDL_PLATFORM_APPLE
#include "ios/iosfirebaseanalytics.h"
#endif

namespace Firebase {
    namespace Analytics {
        void Init() {
#ifdef SDL_PLATFORM_APPLE
Firebase::Analytics::iOS::Init();
#endif
        }
    }
}

#endif