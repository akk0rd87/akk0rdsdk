#pragma once
#ifndef __AKK0RD_SDK_INAPP_BILLING_H__
#define __AKK0RD_SDK_INAPP_BILLING_H__

#include "basewrapper.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class BillingCallbackObserver;

// Товар магазина, как его показать игроку (ответ на QueryProductDetails)
struct BillingProduct
{
    std::string ProductSKU;
    std::string PriceLabel;   // цена с символом валюты, как её показывает магазин ("199 ₽", "$4.99")
    int64_t     PriceMicros;  // цена в миллионных долях валюты (4.99 → 4990000), 0 — неизвестна
    std::string Currency;     // код валюты ISO 4217 ("RUB", "USD"), пусто — неизвестен
    std::string Title;        // название товара в магазине, пусто — неизвестно
};

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

    // Новые события - с пустой реализацией: игре, которой они не нужны, переопределять ничего не надо.

    // Запрос покупок (RestorePurchases и тот, что магазин делает сам при запуске) завершён: все PurchaseUpdatedCallback
    // по нему уже пришли. Success == false - ответа магазина нет (нет сети, пользователь не вошёл в магазин и т.п.),
    // и то, что каких-то покупок не пришло, ничего не значит. Success == true - пришло всё, чем пользователь владеет
    // сейчас (непотребляемые товары; потребляемые - пока не выданы)
    virtual void PurchasesQueryFinishedCallback(bool /*Success*/) {}

    // Магазин сообщил, что товар у пользователя больше не куплен (возврат денег, отзыв покупки). Приходит только там,
    // где магазин говорит об этом сам (iOS, StoreKit 2); в Google Play и RuStore отзыв виден лишь как отсутствие
    // товара в успешном запросе покупок (PurchasesQueryFinishedCallback)
    virtual void PurchaseRevokedCallback(const char* /*ProductSKU*/) {}

    // Ответ на QueryProductDetails: товары, которые магазин знает (товара, которого нет в магазине, нет и в списке;
    // пустой список - магазин не ответил или не знает ни одного)
    virtual void ProductDetailsCallback(const std::vector<BillingProduct>& /*Products*/) {}

    virtual ~BillingCallbackObserver() {}
};

#endif // __AKK0RD_SDK_INAPP_BILLING_H__
