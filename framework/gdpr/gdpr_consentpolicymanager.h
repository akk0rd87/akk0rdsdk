#ifndef __AKK0RD_SDK_GDPR_POLICYMANAGER_H__
#define __AKK0RD_SDK_GDPR_POLICYMANAGER_H__

#include <functional>

namespace GDPRConsentPolicy {
    class Manager {
    public:
        virtual void initialize(std::function<void(void)> callback) {
            callbackFunc = std::move(callback);
        }
    protected:
        Manager() {}
        void onGDPRConsentGathered() {
            callbackFunc();
        }
    private:
        std::function<void(void)> callbackFunc = {};
        Manager(const Manager& rhs) = delete; // Копирующий: конструктор
        Manager(Manager&& rhs) = delete; // Перемещающий: конструктор
        Manager& operator= (const Manager& rhs) = delete; // Оператор копирующего присваивания
        Manager& operator= (Manager&& rhs) = delete; // Оператор перемещающего присваивания
    };

    Manager& getManagerInstance();
}


#endif