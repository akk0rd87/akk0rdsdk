#include <UserMessagingPlatform/UserMessagingPlatform.h>
#include "../gdpr_consentpolicymanager.h"

namespace GDPRConsentPolicy {
    class iOSGDPRManager : public GDPRConsentPolicy::Manager {
    private:
        virtual void initialize(std::function<void(void)> callback) override {
            GDPRConsentPolicy::Manager::initialize(std::move(callback));
        }
    };
}

static GDPRConsentPolicy::iOSGDPRManager iosGDPRManager;

GDPRConsentPolicy::Manager& GDPRConsentPolicy::getManagerInstance() {
    return iosGDPRManager;
}