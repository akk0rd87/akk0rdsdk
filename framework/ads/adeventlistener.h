#ifndef __AKK0RD_SDK_ADS_EVENTLISTENER_H__
#define __AKK0RD_SDK_ADS_EVENTLISTENER_H__

#include "adevent.h"

namespace ads {
    class EventListener {
    public:
        virtual void onAdEvent(ads::Event Event) = 0;
        virtual ~EventListener() = default;
    };
};
#endif