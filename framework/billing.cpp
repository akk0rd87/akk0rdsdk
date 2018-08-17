#include "billing.h"

struct BillingContextStruct
{
	int BillingStatus = -1;
	BillingManager::BillingPurchaseUpdatedCallback* AppPurchaseUpdated = nullptr;
	decltype(SDL_RegisterEvents(1)) BillingEventCode;
};

static BillingContextStruct BillingContext;

int BillingManager::GetStatus()
{
	return BillingContext.BillingStatus;
};

#ifdef __ANDROID__
#include "core/android/android_billing.h"
extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingSetupFinished(JNIEnv*, jclass, jint);
    JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingDisconnected(JNIEnv*, jclass, jint);
	JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_PurchaseQueried(JNIEnv*, jclass, jstring, jstring, jint);
}
JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingSetupFinished(JNIEnv*, jclass, jint ResponseCode)
{
    int Code = (int)ResponseCode;
    logDebug("BillingSetupFinished %d", Code);
    BillingContext.BillingStatus = Code;
}

JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingDisconnected(JNIEnv*, jclass)
{
    logDebug("BillingDisconnected");
}

JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_PurchaseQueried(JNIEnv* env, jclass, jstring PurchaseToken, jstring ProductCode, jint Type)  /* Type: 0 - restored, 1- boufght */
{        
    const char* PToken = env->GetStringUTFChars(PurchaseToken, 0);
    const char* PCode  = env->GetStringUTFChars(ProductCode, 0);
	int ActionType = (int)Type;

	logDebug("PurchaseQueried %s %s %s", PToken, PCode, (ActionType == 0 ? "restored" : "boufght"));
	 
	if(BillingContext.AppPurchaseUpdated)
	{
		BillingContext.AppPurchaseUpdated(PToken, PCode, (BillingManager::OperAction)ActionType);
	}
	else
	{
		logError("AppPurchaseUpdated is not set");
	}

    env->ReleaseStringUTFChars(PurchaseToken, PToken);
    env->ReleaseStringUTFChars(ProductCode, PCode);
}
#endif

#ifdef __APPLE__
#include "core/ios/ios_billing.h"
#endif

////////////////////////////////////////
//////  API
////////////////////////////////////////

bool BillingManager::Init()
{
	BillingContext.BillingEventCode = SDL_RegisterEvents(1);

#ifdef __ANDROID__
    return AndroidBillingManager::Init();
#endif

#ifdef __APPLE__
    return iOSBillingManager::Init();
#endif
    return false;
}

bool BillingManager::QueryProductDetails(const std::vector<std::string>& ProdList)
{
#ifdef __ANDROID__    
    return AndroidBillingManager::QueryProductDetails(ProdList);
#endif

#ifdef __APPLE__    
    return iOSBillingManager::QueryProductDetails(ProdList);
#endif
    return false;
}

bool BillingManager::RestorePurchases()
{
#ifdef __ANDROID__    
    return AndroidBillingManager::RestorePurchases();
#endif

#ifdef __APPLE__    
    return iOSBillingManager::RestorePurchases();
#endif 
    return false;
}

bool BillingManager::PurchaseProdItem(const char* ProductCode)
{
#ifdef __ANDROID__    
    return AndroidBillingManager::PurchaseProdItem(ProductCode);
#endif

    return false;
}

bool BillingManager::ConsumeProductItem(const char* PurchaseToken)
{
#ifdef __ANDROID__    
    return AndroidBillingManager::ConsumeProductItem(PurchaseToken);
#endif

    return false;
}

void BillingManager::SetPurchaseUpdatedCallback(BillingPurchaseUpdatedCallback* Callback)
{
	BillingContext.AppPurchaseUpdated = Callback;
};

decltype(SDL_RegisterEvents(1)) BillingManager::GetEventCode()
{
	return BillingContext.BillingEventCode;
};

void BillingManager::DecodeEvent(const SDL_Event& Event, int& Code, int& Result)
{
	Code = Event.user.code;
	Result = (int)(size_t)Event.user.data1;
};

void BillingManager::PushEvent(const int& Code, const int& Result)
{
	// ѕушим евент в основной поток
	SDL_Event sdl_Event;
	sdl_Event.user.type = BillingManager::GetEventCode();
	sdl_Event.user.code = (Sint32)(Code);
	sdl_Event.user.data1 = (void*)Result;
	SDL_PushEvent(&sdl_Event);
};