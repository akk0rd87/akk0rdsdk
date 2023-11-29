#ifndef __AKK0RD_SDK_GDPR_POLICYOBSERVER_H__
#define __AKK0RD_SDK_GDPR_POLICYOBSERVER_H__

namespace GDPRConsentPolicy {
    class Observer {
    public:
        virtual void onConsentGathered() = 0;
    };
}

#endif