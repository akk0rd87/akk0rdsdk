#include "billing.h"

static int BillingStatus = -1;

int BillingManager::GetStatus()
{
    return BillingStatus;
};

#ifdef __ANDROID__
#include "core/android/android_billing.h"
extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingSetupFinished(JNIEnv*, jclass, jint);
    JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingDisconnected(JNIEnv*, jclass, jint);
	JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_PurchaseQueried(JNIEnv*, jclass, jstring, jstring);
}
JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingSetupFinished(JNIEnv*, jclass, jint ResponseCode)
{
    int Code = (int)ResponseCode;
    logDebug("BillingSetupFinished %d", Code);
    BillingStatus = Code;
}

JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingDisconnected(JNIEnv*, jclass)
{
    logDebug("BillingDisconnected");
}

JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_PurchaseQueried(JNIEnv* env, jclass, jstring PurchaseToken, jstring ProductCode)
{		
	const char* PToken = env->GetStringUTFChars(PurchaseToken, 0);
	const char* PCode  = env->GetStringUTFChars(ProductCode, 0);

	logDebug("PurchaseQueried %s %s", PToken, PCode);

	env->ReleaseStringUTFChars(PurchaseToken, PToken);
	env->ReleaseStringUTFChars(ProductCode, PCode);
}
#endif

#ifdef __APPLE__
#include "core/ios/ios_billing.h"
#endif

bool BillingManager::Init()
{
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

#ifdef __APPLE__    
	return iOSBillingManager::RestorePurchases();
#endif 
	return false;
}

bool BillingManager::ConsumeProductItem(const char* PurchaseToken)
{
#ifdef __ANDROID__    
	return AndroidBillingManager::ConsumeProductItem(PurchaseToken);
#endif

#ifdef __APPLE__    
	return AndroidBillingManager::RestorePurchases();
#endif 
	return false;
}