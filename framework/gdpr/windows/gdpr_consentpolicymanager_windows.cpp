#include "../gdpr_consentpolicymanager.h"

namespace GDPRConsentPolicy {
    class WindowsGDPRManager : public GDPRConsentPolicy::Manager {
    private:
        virtual void requestConsent() override {
            onGDPRConsentGathered(); // on Windows we imitate consent gather immediate
        }

        virtual void showPrivacyOptionsForm() override {

        }
    };
}

static GDPRConsentPolicy::WindowsGDPRManager windowsGDPRManager;

GDPRConsentPolicy::Manager& GDPRConsentPolicy::getManagerInstance() {
    return windowsGDPRManager;
}