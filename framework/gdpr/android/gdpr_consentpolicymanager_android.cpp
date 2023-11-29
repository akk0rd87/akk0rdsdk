#include "../gdpr_consentpolicymanager.h"

class AndroidGDPRManager : public GDPRConsentPolicy::Manager {
public:
    void onConsentGatheredFromJava() {
        onConsentGathered();
    }
private:
    virtual void initialize(GDPRConsentPolicy::Observer* observer) override {
        GDPRConsentPolicy::Manager::initialize(observer);
    }
};

static AndroidGDPRManager androidGDPRManager;

GDPRConsentPolicy::Manager& getManagerInstance() {
    return androidGDPRManager;
}