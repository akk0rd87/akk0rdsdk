#include "../gdpr_consentpolicymanager.h"

class WindowsGDPRManager : public GDPRConsentPolicy::Manager {
private:
    virtual void initialize(std::function<void(void)> callback) override {
        GDPRConsentPolicy::Manager::initialize(std::move(callback));
        onGDPRConsentGathered(); // on Windows we imitate consent gather immediate
    }
};

static WindowsGDPRManager windowsGDPRManager;

GDPRConsentPolicy::Manager& GDPRConsentPolicy::getManagerInstance() {
    return windowsGDPRManager;
}