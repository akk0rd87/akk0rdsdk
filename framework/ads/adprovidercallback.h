#ifndef __AKK0RD_SDK_ADS_PROVIDERCALLBACK_H__
#define __AKK0RD_SDK_ADS_PROVIDERCALLBACK_H__

#include <chrono>
#include "adevent.h"

namespace ads {
    using timeMS = decltype(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    class ProviderCallback {
    public:
        virtual void eventCallback(int Event) const = 0;
        virtual ~ProviderCallback() = default;
    protected:
        timeMS GetSeconds() const {
            return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        };
    };
};
#endif