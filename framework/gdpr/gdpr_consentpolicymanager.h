#ifndef __AKK0RD_SDK_GDPR_POLICYMANAGER_H__
#define __AKK0RD_SDK_GDPR_POLICYMANAGER_H__

#include <functional>

namespace GDPRConsentPolicy {
    class Manager {
    public:
        void initialize(
            std::function<void(void)> consentGatheredCallback,
            std::function<void(void)> privacyOptionsRequiredCallback
        ) {
            consentGatheredFunc = std::move(consentGatheredCallback);
            privacyOptionsRequiredFunc = std::move(privacyOptionsRequiredCallback);
            requestConsent();
        }

        virtual void showPrivacyOptionsForm() = 0;
    protected:
        Manager() {}
        void onGDPRConsentGathered() {
            consentGatheredFunc();
        }

        void setPrivacyOptionsRequired() {
            privacyOptionsRequiredFunc();
        }

        virtual void requestConsent() = 0;
    private:
        std::function<void(void)> consentGatheredFunc = {};
        std::function<void(void)> privacyOptionsRequiredFunc = {};
        Manager(const Manager& rhs) = delete; // Копирующий: конструктор
        Manager(Manager&& rhs) = delete; // Перемещающий: конструктор
        Manager& operator= (const Manager& rhs) = delete; // Оператор копирующего присваивания
        Manager& operator= (Manager&& rhs) = delete; // Оператор перемещающего присваивания
    };

    Manager& getManagerInstance();
}


#endif