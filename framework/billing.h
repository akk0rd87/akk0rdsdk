#pragma once
#ifndef __AKK0RD_SDK_INAPP_BILLING_H__
#define __AKK0RD_SDK_INAPP_BILLING_H__

#include "basewrapper.h"
#include <functional>
class BillingManager
{
public:
    enum struct OperAction : int { Restored = 0, Bought = 1 };

    using PurchaseUpdatedCallbackFunction = std::function<void(const char* PurchaseToken, const char* SKUid, BillingManager::OperAction Action)>;
    using PurchaseUpdatedConsumedFunction = std::function<void(const char* PurchaseToken)>;

    static bool                             Init();
    static int                              GetStatus();
    static bool                             QueryProductDetails(const std::vector<std::string>& ProdList);
    static bool                             RestorePurchases();
    static bool                             PurchaseProdItem(const char* ProductCode);
    static bool                             ConsumeProductItem(const char* PurchaseToken);

    static void                             SetPurchaseUpdatedCallback(const PurchaseUpdatedCallbackFunction& Function);
    static void                             SetPurchaseConsumedCallback(const PurchaseUpdatedConsumedFunction& Function);

    static decltype(SDL_RegisterEvents(1))  GetEventCode();

    static void                             DecodeEvent(const SDL_Event& Event, int& Code, int& Result);
    static void                             PushEvent(int Code, const int Result);

    //Запрещаем создавать экземпляр класса BillingManager
    BillingManager() = delete;
    ~BillingManager() = delete;
    BillingManager(const BillingManager& rhs) = delete; // Копирующий: конструктор
    BillingManager(BillingManager&& rhs) = delete; // Перемещающий: конструктор
    BillingManager& operator= (const BillingManager& rhs) = delete; // Оператор копирующего присваивания
    BillingManager& operator= (BillingManager&& rhs) = delete; // Оператор перемещающего присваивания
};

#endif // __AKK0RD_SDK_INAPP_BILLING_H__
