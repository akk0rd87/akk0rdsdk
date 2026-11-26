// iOSBillingManager на StoreKit 2 (StoreKit2Billing.swift): вместо core/platformwrapper/ios/ios_billing.mm - приложение
// подключает одно из двух. ЧЕРНОВИК: на Mac ещё не собирался (см. StoreKit2Billing.swift).

#include "core/platformwrapper/ios/ios_billing.h"
#include "storekit2_bridge.h"

#include <string>
#include <vector>

namespace {

BillingCallbackObserver* observer = nullptr;
std::vector<BillingProduct> productsPending;  // товары ответа, пока не пришёл OnProductsDone

void onPurchase(const char* Token, const char* ProductSKU, int32_t Action) {
    if (observer) {
        observer->PurchaseUpdatedCallback(Token, ProductSKU,
            Action == 1 ? BillingManager::OperAction::Bought : BillingManager::OperAction::Restored);
    }
}

void onConsumed(const char* Token, const char* ProductSKU) {
    if (observer) observer->PurchaseConsumedCallback(Token, ProductSKU);
}

void onQueryFinished(int32_t Success) {
    if (observer) observer->PurchasesQueryFinishedCallback(Success != 0);
}

void onRevoked(const char* ProductSKU) {
    if (observer) observer->PurchaseRevokedCallback(ProductSKU);
}

void onProduct(const char* ProductSKU, const char* PriceLabel, int64_t PriceMicros, const char* Currency, const char* Title) {
    productsPending.push_back(BillingProduct{ ProductSKU, PriceLabel, PriceMicros, Currency, Title });
}

void onProductsDone() {
    std::vector<BillingProduct> Products;
    Products.swap(productsPending);
    if (observer) observer->ProductDetailsCallback(Products);
}

} // namespace

bool iOSBillingManager::Init(BillingCallbackObserver* Observer)
{
    observer = Observer;
    akk_sk2_start(onPurchase, onConsumed, onQueryFinished, onRevoked, onProduct, onProductsDone);
    return true;
}

bool iOSBillingManager::QueryProductDetails(const std::vector<std::string>& ProdList)
{
    std::vector<const char*> Skus;
    Skus.reserve(ProdList.size());
    for (const auto& Sku : ProdList) Skus.push_back(Sku.c_str());
    akk_sk2_query_products(Skus.data(), static_cast<int32_t>(Skus.size()));
    return true;
}

bool iOSBillingManager::RestorePurchases()
{
    akk_sk2_restore();
    return true;
}

bool iOSBillingManager::PurchaseProdItem(const char* ProductCode)
{
    akk_sk2_purchase(ProductCode);
    return true;
}

bool iOSBillingManager::ConsumeProductItem(const char* PurchaseToken)
{
    akk_sk2_consume(PurchaseToken);
    return true;
}
