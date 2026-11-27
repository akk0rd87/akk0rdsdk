package org.akkord.lib

import android.content.Intent
import android.util.Log
import com.samsung.android.sdk.iap.lib.constants.HelperDefine

// Биллинг Samsung Galaxy Store на Samsung IAP SDK (com.samsung.developer:iap). Нужен Galaxy Store на устройстве.
// Приложению: зависимость libs.samsung.iap, в манифесте флейвора разрешение
//   <uses-permission android:name="com.samsung.android.iap.permission.BILLING"/>
// (в AAR его нет) и, для проверки без оплаты, akk0rdsdk.samsung.iap.mode (см. SamsungGateway.kt); товары - в Seller
// Portal; ProGuard - framework/core/platformwrapper/android/proguard/proguard-rules_samsung.pro.
// Товары: непотребляемые - restorePurchases сообщает о них при каждом вызове; потребляемые - пока не выданы
// (consumeProductItem); подписок нет. Подтверждать (acknowledge) разовые товары Samsung не требует.
// Здесь только решения, вызовы Samsung IAP - в SamsungGateway (SamsungIapGateway); тесты -
// ../samsung_test/BillingImplementationTest.kt.
class BillingImplementation(
    val observer: BillingObserver,
    private val store: SamsungGateway = SamsungIapGateway(),
) {
    companion object {
        const val PURCHASE_RESTORED = 0
        const val PURCHASE_BOUGHT   = 1
    }

    // Отдельного подключения у Samsung IAP нет: готовность - сразу (дальше BillingManager восстанавливает покупки)
    fun init() {
        observer.onBillingSetupFinished()
    }

    fun restorePurchases() {
        store.ownedItems { code, purchases ->
            if (HelperDefine.IAP_ERROR_NONE == code) {
                for (purchase in purchases) report(purchase, PURCHASE_RESTORED)
            } else {
                log("getOwnedList: $code")
            }
            observer.onPurchasesQueryFinished(HelperDefine.IAP_ERROR_NONE == code)
        }
    }

    fun purchaseProdItem(productSKU: String) {
        store.purchase(productSKU) { code, purchase ->
            when (code) {
                HelperDefine.IAP_ERROR_NONE -> purchase?.let { report(it, PURCHASE_BOUGHT) }
                // Куплено раньше (на другом устройстве, или выдача не дошла): узнать из восстановления
                HelperDefine.IAP_ERROR_ALREADY_PURCHASED -> restorePurchases()
                HelperDefine.IAP_PAYMENT_IS_CANCELED -> {}
                else -> log("startPayment $productSKU: $code")
            }
        }
    }

    fun purchaseSubscription(productSKU: String) {
    }

    fun consumeProductItem(purchaseToken: String, productSKU: String) {
        store.consume(purchaseToken) { code ->
            if (HelperDefine.IAP_ERROR_NONE == code) observer.onPurchaseConsumed(purchaseToken, productSKU)
            else log("consumePurchasedItems $productSKU: $code")
        }
    }

    fun onNewIntent(intent: Intent) {
    }

    // Товары с ценами -> onProductDetails (пустой список - Galaxy Store не ответил)
    fun queryProductDetails(prodList: Array<String?>?) {
        store.products(prodList.orEmpty().filterNotNull().filter { it.isNotEmpty() }) { code, products ->
            if (HelperDefine.IAP_ERROR_NONE != code) log("getProductsDetails: $code")
            observer.onProductDetails(if (HelperDefine.IAP_ERROR_NONE == code) products else emptyList())
        }
    }

    fun querySubscriptionProducts(prodList: Array<String>) {
    }

    private fun report(purchase: SamsungPurchase, boughtOrRestored: Int) {
        observer.onPurchaseQueried(purchase.purchaseId, purchase.productSKU, boughtOrRestored)
    }

    private fun log(message: String) {
        Log.d(Utils.TAG, "Samsung IAP: $message")
    }
}
