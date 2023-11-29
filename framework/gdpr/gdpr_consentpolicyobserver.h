#ifndef __AKK0RD_SDK_GDPR_POLICYOBSERVER_H__
#define __AKK0RD_SDK_GDPR_POLICYOBSERVER_H__

namespace GDPRConsentPolicy {
    class Observer {
    public:
        virtual void onGDPRConsentGathered() = 0;
        virtual ~Observer() {}
    };
}

#endif