#ifndef __AKK0RD_SDK_GDPR_POLICYMANAGER_H__
#define __AKK0RD_SDK_GDPR_POLICYMANAGER_H__

#include "gdpr_consentpolicyobserver.h"

namespace GDPRConsentPolicy {
    class Manager {
    public:
        virtual void initialize(GDPRConsentPolicy::Observer* observer) {
            callbackObserver = observer;
        }
    protected:
        void onConsentGathered() {
            if (callbackObserver) {
                callbackObserver->onConsentGathered();
            }
        }
    private:
        GDPRConsentPolicy::Observer* callbackObserver{ nullptr };
    };

    Manager& getManagerInstance();
}


#endif