#include "basewrapper.h"
#include <jni.h>
#include "../gdpr_consentpolicymanager.h"

namespace GDPRConsentPolicy {
    class AndroidGDPRManager : public GDPRConsentPolicy::Manager {
    public:
        void onGDPRConsentGatheredFromJava() {
            onGDPRConsentGathered();
        }

        void setPrivacyOptionsRequiredFromJava() {
            setPrivacyOptionsRequired();
        }
    private:
        JNIEnv* getJNIEnv() {
            return (JNIEnv*)SDL_AndroidGetJNIEnv();
        }

        jclass getGDPRManager() {
            auto env = getJNIEnv();
            jclass gdprManager = env->FindClass("org/akkord/lib/GDPRConsentPolicyManager");
            if (!gdprManager) {
                logError("gdprManager not Found");
            }
            return gdprManager;
        }

        virtual void requestConsent() override {
            auto gdprManager = getGDPRManager();
            if (gdprManager) {
                auto env = getJNIEnv();
                jmethodID initMethod = env->GetStaticMethodID(gdprManager, "Initialize", "()V");
                if (!initMethod) {
                    logError("Initialize Java method not Found");
                    return;
                }
                env->CallStaticVoidMethod(gdprManager, initMethod);
            }
        }

        virtual void showPrivacyOptionsForm() override {
            auto gdprManager = getGDPRManager();
            if (gdprManager) {
                auto env = getJNIEnv();
                jmethodID showPrivacyOptions = env->GetStaticMethodID(gdprManager, "ShowPrivacyOptionsForm", "()V");
                if (!showPrivacyOptions) {
                    logError("ShowPrivacyOptionsForm Java method not Found");
                    return;
                }
                env->CallStaticVoidMethod(gdprManager, showPrivacyOptions);
            }
        }
    };
}

static GDPRConsentPolicy::AndroidGDPRManager androidGDPRManager;

GDPRConsentPolicy::Manager& GDPRConsentPolicy::getManagerInstance() {
    return androidGDPRManager;
}

extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_GDPRConsentPolicyManager_GDPRCallback(JNIEnv*, jclass, jint Code) {
        switch (Code) {
        case 0: androidGDPRManager.onGDPRConsentGatheredFromJava(); break;
        case 1: androidGDPRManager.setPrivacyOptionsRequiredFromJava(); break;
        default:
            break;
        }
    }
}