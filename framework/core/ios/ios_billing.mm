#include "ios_billing.h"

#import <StoreKit/StoreKit.h>

bool iOSBillingManager::Init()
{
    return true;
};

bool iOSBillingManager::RestorePurchases()
{
    return true;
};

bool iOSBillingManager::PurchaseProdItem(const char* ProductCode)
{
    return true;
};

bool iOSBillingManager::ConsumeProductItem(const char* PurchaseToken)
{
    return true;
};

bool iOSBillingManager::QueryProductDetails(const std::vector<std::string>& ProdList)
{
    return true;
};
