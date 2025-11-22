#pragma once
#ifndef __AKK0RD_SDK_INAPP_BILLING_H__
#define __AKK0RD_SDK_INAPP_BILLING_H__

#include "basewrapper.h"
#include <functional>

class BillingCallbackObserver;

class BillingManager
{
public:
    enum struct OperAction : int { Restored = 0, Bought = 1 };

    static bool                             Init(BillingCallbackObserver* Observer);
    static bool                             QueryProductDetails(const std::vector<std::string>& ProductSKUList);
    static bool                             RestorePurchases();
    static bool                             PurchaseProdItem(const char* ProductSKU);
    static bool                             ConsumeProductItem(const char* PurchaseToken, const char* ProductSKU);

    static decltype(SDL_RegisterEvents(1))  GetEventCode();

    static void                             DecodeEvent(const SDL_Event& Event, int& Code, int& Result);
    static void                             PushEvent(int Code, const int Result);
    static void                             ClearObserver();

    //Запрещаем создавать экземпляр класса BillingManager
    BillingManager() = delete;
    ~BillingManager() = delete;
    BillingManager(const BillingManager& rhs) = delete; // Копирующий: конструктор
    BillingManager(BillingManager&& rhs) = delete; // Перемещающий: конструктор
    BillingManager& operator= (const BillingManager& rhs) = delete; // Оператор копирующего присваивания
    BillingManager& operator= (BillingManager&& rhs) = delete; // Оператор перемещающего присваивания
};

class BillingCallbackObserver {
public:
    virtual void PurchaseUpdatedCallback(const char* PurchaseToken, const char* ProductSKU, BillingManager::OperAction Action) = 0;
    virtual void PurchaseConsumedCallback(const char* PurchaseToken, const char* ProductSKU) = 0;

    virtual ~BillingCallbackObserver() {}
};

#endif // __AKK0RD_SDK_INAPP_BILLING_H__
