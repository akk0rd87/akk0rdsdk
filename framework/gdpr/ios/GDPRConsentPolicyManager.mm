#include <UserMessagingPlatform/UserMessagingPlatform.h>
#include "../gdpr_consentpolicymanager.h"

namespace GDPRConsentPolicy {
    class iOSGDPRManager : public GDPRConsentPolicy::Manager {
    private:
        virtual void requestConsent() override {
        }
    };
}

static GDPRConsentPolicy::iOSGDPRManager iosGDPRManager;

GDPRConsentPolicy::Manager& GDPRConsentPolicy::getManagerInstance() {
    return iosGDPRManager;
}