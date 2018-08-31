#pragma once
#ifndef __AKK0RD_INAPP_BILLING_H__
#define __AKK0RD_INAPP_BILLING_H__

#include "basewrapper.h"

class BillingManager
{
public:
	enum struct OperAction : int { Restored = 0, Bought = 1 };

	typedef void (BillingPurchaseUpdatedCallback)(const char* PurchaseToken, const char* SKUid, BillingManager::OperAction Action);
	typedef void (BillingPurchaseConsumedCallback)(const char* PurchaseToken);
	

	static bool                             Init();
    static int                              GetStatus();
    static bool                             QueryProductDetails(const std::vector<std::string>& ProdList);
    static bool                             RestorePurchases();
    static bool                             PurchaseProdItem(const char* ProductCode);
    static bool                             ConsumeProductItem(const char* PurchaseToken);
	
	static void                             SetPurchaseUpdatedCallback (BillingPurchaseUpdatedCallback * Callback);
	static void                             SetPurchaseConsumedCallback(BillingPurchaseConsumedCallback* Callback);
	
	static decltype(SDL_RegisterEvents(1))  GetEventCode();
	
	static void                             DecodeEvent(const SDL_Event& Event, int& Code, int& Result);
	static void                             PushEvent  (int Code, const int Result);
};

#endif // __AKK0RD_INAPP_BILLING_H__
