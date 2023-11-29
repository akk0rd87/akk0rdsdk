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
        Manager() {}
        void onGDPRConsentGathered() {
            if (callbackObserver) {
                callbackObserver->onGDPRConsentGathered();
            }
        }
    private:
        GDPRConsentPolicy::Observer* callbackObserver{ nullptr };

        Manager(const Manager& rhs) = delete; // Копирующий: конструктор
        Manager(Manager&& rhs) = delete; // Перемещающий: конструктор
        Manager& operator= (const Manager& rhs) = delete; // Оператор копирующего присваивания
        Manager& operator= (Manager&& rhs) = delete; // Оператор перемещающего присваивания

    };

    Manager& getManagerInstance();
}


#endif