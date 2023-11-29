#include "basewrapper.h"
#include <jni.h>
#include "../gdpr_consentpolicymanager.h"

class AndroidGDPRManager : public GDPRConsentPolicy::Manager {
public:
    void onGDPRConsentGatheredFromJava() {
        onGDPRConsentGathered();
    }
private:
    JNIEnv* getJNIEnv() {
        return (JNIEnv*)SDL_AndroidGetJNIEnv();
    }

    virtual void initialize(GDPRConsentPolicy::Observer* observer) override {
        GDPRConsentPolicy::Manager::initialize(observer);

        auto env = getJNIEnv();
        jclass gdprManager = env->FindClass("org/akkord/lib/GDPRConsentPolicyManager");
        if (!gdprManager) {
            logError("gdprManager not Found");
            return;
        }

        jmethodID initMethod = env->GetStaticMethodID(gdprManager, "Initialize", "()V");
        if (!initMethod) {
            logError("Initialize Java method not Found");
            return;
        }

        env->CallStaticVoidMethod(gdprManager, initMethod);
    }
};

static AndroidGDPRManager androidGDPRManager;

GDPRConsentPolicy::Manager& GDPRConsentPolicy::getManagerInstance() {
    return androidGDPRManager;
}

extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_GDPRConsentPolicyManager_GDPRConsentReceived(JNIEnv*, jclass) {
        androidGDPRManager.onGDPRConsentGatheredFromJava();
    }
}