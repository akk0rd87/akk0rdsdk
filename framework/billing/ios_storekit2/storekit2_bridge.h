#pragma once
#ifndef __AKK0RD_SDK_STOREKIT2_BRIDGE_H__
#define __AKK0RD_SDK_STOREKIT2_BRIDGE_H__

// C-функции StoreKit2Billing.swift (@_cdecl) для ios_billing_storekit2.cpp. Типы должны совпадать с typealias
// AkkSk2*Fn в StoreKit2Billing.swift: Swift не видит этот заголовок и проверить их не может.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*AkkSk2PurchaseFn)(const char* Token, const char* ProductSKU, int32_t Action); // 0 restored, 1 bought
typedef void (*AkkSk2ConsumedFn)(const char* Token, const char* ProductSKU);
typedef void (*AkkSk2QueryFinishedFn)(int32_t Success);
typedef void (*AkkSk2RevokedFn)(const char* ProductSKU);
typedef void (*AkkSk2ProductFn)(const char* ProductSKU, const char* PriceLabel, int64_t PriceMicros,
                                const char* Currency, const char* Title);
typedef void (*AkkSk2ProductsDoneFn)(void);

void akk_sk2_start(AkkSk2PurchaseFn OnPurchase, AkkSk2ConsumedFn OnConsumed, AkkSk2QueryFinishedFn OnQueryFinished,
                   AkkSk2RevokedFn OnRevoked, AkkSk2ProductFn OnProduct, AkkSk2ProductsDoneFn OnProductsDone);
void akk_sk2_query_products(const char* const* ProductSKUs, int32_t Count);
void akk_sk2_restore(void);
void akk_sk2_purchase(const char* ProductSKU);
void akk_sk2_consume(const char* Token);

#ifdef __cplusplus
}
#endif

#endif // __AKK0RD_SDK_STOREKIT2_BRIDGE_H__
