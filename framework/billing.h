#pragma once
#ifndef __AKK0RD_INAPP_BILLING_H__
#define __AKK0RD_INAPP_BILLING_H__

#include "basewrapper.h"

class BillingManager
{
public:
	enum struct OperAction : int { Restored = 0, Boufght = 1 };

	typedef void (BillingPurchaseUpdatedCallback)(const char* PurchaseToken, const char* SKUid, BillingManager::OperAction Action);
	

	static bool                 Init();
    static int                  GetStatus();
    static bool                 QueryProductDetails(const std::vector<std::string>& ProdList);
    static bool                 RestorePurchases();
    static bool                 PurchaseProdItem(const char* ProductCode);
    static bool                 ConsumeProductItem(const char* PurchaseToken);

	
	static void                 SetPurchaseUpdatedCallback(BillingPurchaseUpdatedCallback* Callback);
};

#endif // __AKK0RD_INAPP_BILLING_H__
