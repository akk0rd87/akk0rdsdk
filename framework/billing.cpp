#include "billing.h"

struct BillingContextStruct
{
    int BillingStatus = -1;
    decltype(SDL_RegisterEvents(1)) BillingEventCode;

#if (__ANDROID__) || (__WINDOWS__)
    BillingCallbackObserver* callbackObserver = nullptr;
#endif
};

static BillingContextStruct BillingContext;

int BillingManager::GetStatus()
{
    return BillingContext.BillingStatus;
};

#ifdef __ANDROID__
#include "core/platformwrapper/android/android_billing.h"
extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingSetupFinished(JNIEnv*, jclass, jint);
    JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingDisconnected(JNIEnv*, jclass);
    JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_PurchaseQueried(JNIEnv*, jclass, jstring, jstring, jint);
    JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_PurchaseConsumed(JNIEnv*, jclass, jstring, jstring);
}
JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingSetupFinished(JNIEnv*, jclass, jint ResponseCode)
{
    int Code = (int)ResponseCode;
    //logDebug("BillingSetupFinished %d", Code);
    BillingContext.BillingStatus = Code;
}

JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_BillingDisconnected(JNIEnv*, jclass)
{
    //logDebug("BillingDisconnected");
}

JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_PurchaseQueried(JNIEnv* env, jclass, jstring PurchaseToken, jstring ProductCode, jint Type)  /* Type: 0 - restored, 1- bought */
{
    const char* PToken = env->GetStringUTFChars(PurchaseToken, 0);
    const char* PCode = env->GetStringUTFChars(ProductCode, 0);
    int ActionType = (int)Type;

    logDebug("PurchaseQueried %s %s %s", PToken, PCode, (ActionType == 0 ? "restored" : "bought"));

    if (BillingContext.callbackObserver)
    {
        BillingContext.callbackObserver->PurchaseUpdatedCallback(PToken, PCode, (BillingManager::OperAction)ActionType);
    }
    else
    {
        logError("updateCallBackFunction is not set");
    }

    env->ReleaseStringUTFChars(PurchaseToken, PToken);
    env->ReleaseStringUTFChars(ProductCode, PCode);
}

JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManager_PurchaseConsumed(JNIEnv* env, jclass, jstring PurchaseToken, jstring ProductSKU)
{
    const char* purchToken = env->GetStringUTFChars(PurchaseToken, 0);
    const char* prodToken  = env->GetStringUTFChars(ProductSKU   , 0);

    if (BillingContext.callbackObserver) {
        BillingContext.callbackObserver->PurchaseConsumedCallback(purchToken, prodToken);
    }
    else {
        logError("consumedCallBackFunction is not set");
    }

    env->ReleaseStringUTFChars(PurchaseToken, purchToken);
    env->ReleaseStringUTFChars(ProductSKU   , prodToken );
}
#endif

#ifdef __APPLE__
#include "core/platformwrapper/ios/ios_billing.h"
#endif

////////////////////////////////////////
//////  API
////////////////////////////////////////

bool BillingManager::Init(BillingCallbackObserver* Observer)
{
    BillingContext.BillingEventCode = SDL_RegisterEvents(1);

#ifdef __ANDROID__
    BillingContext.callbackObserver = Observer;
    return AndroidBillingManager::Init();
#endif

#ifdef __APPLE__
    return iOSBillingManager::Init(Observer);
#endif

#ifdef __WINDOWS__
    BillingContext.callbackObserver = Observer;
    return true; // будем считать, что на винде все прошло норм
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
    return iOSBillingManager::PurchaseProdItem(ProductCode);
#endif

#ifdef __WINDOWS__
    // на винде на десктопе в дебаг режиме всега подтверждаем покупку Callback-ом
    if (!BWrapper::IsReleaseBuild()) {
        BillingContext.callbackObserver->PurchaseUpdatedCallback("testPurchaseToken", ProductCode, BillingManager::OperAction::Bought);
        return true;
    }
#endif

    return false;
}

bool BillingManager::ConsumeProductItem(const char* PurchaseToken, const char* ProductCode)
{
#ifdef __ANDROID__
    return AndroidBillingManager::ConsumeProductItem(PurchaseToken, ProductCode);
#endif

    return false;
}

decltype(SDL_RegisterEvents(1)) BillingManager::GetEventCode()
{
    return BillingContext.BillingEventCode;
};

void BillingManager::DecodeEvent(const SDL_Event& Event, int& Code, int& Result)
{
    Code = Event.user.code;
    Result = (int)(size_t)Event.user.data1;
};

void BillingManager::PushEvent(int Code, int Result)
{
    // ѕушим евент в основной поток
    SDL_Event sdl_Event;
    sdl_Event.user.type = BillingManager::GetEventCode();
    sdl_Event.user.code = (Sint32)(Code);
    sdl_Event.user.data1 = (void*)(uintptr_t)Result;
    SDL_PushEvent(&sdl_Event);
};