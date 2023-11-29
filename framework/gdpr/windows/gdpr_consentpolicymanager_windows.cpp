#include "../gdpr_consentpolicymanager.h"

class WindowsGDPRManager : public GDPRConsentPolicy::Manager {
private:
    virtual void initialize(GDPRConsentPolicy::Observer* observer) override {
        GDPRConsentPolicy::Manager::initialize(observer);
        onGDPRConsentGathered(); // on Windows we imitate consent gather immediate
    }
};

static WindowsGDPRManager windowsGDPRManager;

GDPRConsentPolicy::Manager& GDPRConsentPolicy::getManagerInstance() {
    return windowsGDPRManager;
}