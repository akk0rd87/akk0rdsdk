#include "billing.h"

struct BillingContextStruct
{
    decltype(SDL_RegisterEvents(1)) BillingEventCode;

#if (__ANDROID__) || (SDL_PLATFORM_WINDOWS)
    BillingCallbackObserver* callbackObserver = nullptr;
#endif
};

static BillingContextStruct BillingContext;

#ifdef __ANDROID__
#include "core/platformwrapper/android/android_billing.h"
#include "core/platformwrapper/android/android_wrapper.h"

static JNIEnv* getJNIEnv() {
    return AndroidWrapper::GetJNIEnv();
}

extern "C" {
	JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManagerKt_billingSetupFinished(JNIEnv*, jclass)
	{
		logDebug("billingSetupFinished");
	}

	JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManagerKt_billingDisconnected(JNIEnv*, jclass)
	{
		logDebug("billingSetupFinished");
	}

	JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManagerKt_purchaseQueried(JNIEnv* env, jclass, jstring PurchaseToken, jstring ProductCode, jint Type)  /* Type: 0 - restored, 1- bought */
	{
		const char* PToken = env->GetStringUTFChars(PurchaseToken, 0);
		const char* PCode = env->GetStringUTFChars(ProductCode, 0);
		int ActionType = Type;

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

	JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManagerKt_purchaseConsumed(JNIEnv* env, jclass, jstring PurchaseToken, jstring ProductSKU)
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

	JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManagerKt_purchasesQueryFinished(JNIEnv*, jclass, jboolean Success)
	{
		logDebug("purchasesQueryFinished %d", (int)Success);
		if (BillingContext.callbackObserver) {
			BillingContext.callbackObserver->PurchasesQueryFinishedCallback(Success == JNI_TRUE);
		}
	}

	// Массивы одной длины: товар i - i-е элементы каждого
	JNIEXPORT void JNICALL Java_org_akkord_lib_BillingManagerKt_productDetailsReceived(JNIEnv* env, jclass,
		jobjectArray Skus, jobjectArray PriceLabels, jlongArray PriceMicros, jobjectArray Currencies, jobjectArray Titles)
	{
		const auto stringAt = [env](jobjectArray Array, jsize Index) {
			std::string Result;
			if (auto Value = static_cast<jstring>(env->GetObjectArrayElement(Array, Index))) {
				if (const char* Chars = env->GetStringUTFChars(Value, nullptr)) {
					Result = Chars;
					env->ReleaseStringUTFChars(Value, Chars);
				}
				env->DeleteLocalRef(Value);
			}
			return Result;
		};

		const jsize Count = env->GetArrayLength(Skus);
		std::vector<jlong> Micros(static_cast<size_t>(Count));
		if (Count > 0) {
			env->GetLongArrayRegion(PriceMicros, 0, Count, Micros.data());
		}

		std::vector<BillingProduct> Products;
		Products.reserve(static_cast<size_t>(Count));
		for (jsize i = 0; i < Count; ++i) {
			Products.push_back(BillingProduct{ stringAt(Skus, i), stringAt(PriceLabels, i),
				static_cast<int64_t>(Micros[static_cast<size_t>(i)]), stringAt(Currencies, i), stringAt(Titles, i) });
		}

		logDebug("productDetailsReceived %d", (int)Count);
		if (BillingContext.callbackObserver) {
			BillingContext.callbackObserver->ProductDetailsCallback(Products);
		}
	}
}
#endif

#ifdef SDL_PLATFORM_IOS
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
    return AndroidBillingManager::Init(getJNIEnv());
#endif

#ifdef SDL_PLATFORM_IOS
    return iOSBillingManager::Init(Observer);
#endif

#ifdef SDL_PLATFORM_WINDOWS
    BillingContext.callbackObserver = Observer;
    return true; // будем считать, что на винде все прошло норм
#endif

    return false;
}

bool BillingManager::QueryProductDetails(const std::vector<std::string>& ProdList)
{
#ifdef __ANDROID__
    return AndroidBillingManager::QueryProductDetails(getJNIEnv(), ProdList);
#endif

#ifdef SDL_PLATFORM_IOS
    return iOSBillingManager::QueryProductDetails(ProdList);
#endif
    return false;
}

bool BillingManager::RestorePurchases()
{
#ifdef __ANDROID__
    return AndroidBillingManager::RestorePurchases(getJNIEnv());
#endif

#ifdef SDL_PLATFORM_IOS
    return iOSBillingManager::RestorePurchases();
#endif
    return false;
}

bool BillingManager::PurchaseProdItem(const char* ProductCode)
{
#ifdef __ANDROID__
    return AndroidBillingManager::PurchaseProdItem(getJNIEnv(), ProductCode);
#endif

#ifdef SDL_PLATFORM_IOS
    return iOSBillingManager::PurchaseProdItem(ProductCode);
#endif

#ifdef SDL_PLATFORM_WINDOWS
    // на винде на десктопе в дебаг режиме всега подтверждаем покупку Callback-ом
    if (!BWrapper::IsReleaseBuild() && BillingContext.callbackObserver) {
        BillingContext.callbackObserver->PurchaseUpdatedCallback("testPurchaseToken", ProductCode, BillingManager::OperAction::Bought);
        return true;
    }
#endif

    return false;
}

bool BillingManager::ConsumeProductItem(const char* PurchaseToken, const char* ProductCode)
{
#ifdef __ANDROID__
    return AndroidBillingManager::ConsumeProductItem(getJNIEnv(), PurchaseToken, ProductCode);
#endif

#ifdef SDL_PLATFORM_IOS
    // StoreKit 2 (billing/ios_storekit2) закрывает здесь потребляемую покупку; StoreKit 1 (ios_billing.mm) закрывает
    // её сразу при покупке, у него это пустой вызов
    (void)ProductCode;
    return iOSBillingManager::ConsumeProductItem(PurchaseToken);
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

void BillingManager::ClearObserver() {
#if (__ANDROID__) || (SDL_PLATFORM_WINDOWS)
    BillingContext.callbackObserver = nullptr;
#endif
}