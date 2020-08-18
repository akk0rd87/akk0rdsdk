#pragma once
#ifndef __AKK0RD_SDK_IOS_INAPP_BILLING_H__
#define __AKK0RD_SDK_IOS_INAPP_BILLING_H__

#include "basewrapper.h"
#include "billing.h"
#include "core/core_defines.h"

class iOSBillingManager
{
public:
    static bool Init();
    static bool QueryProductDetails(const std::vector<std::string>& ProdList);
    static bool RestorePurchases();
    static bool PurchaseProdItem(const char* ProductCode);
    static bool ConsumeProductItem(const char* PurchaseToken);
    static void SetPurchaseUpdatedCallback (BillingManager::BillingPurchaseUpdatedCallback * Callback);
};

#endif // __AKK0RD_SDK_IOS_INAPP_BILLING_H__
