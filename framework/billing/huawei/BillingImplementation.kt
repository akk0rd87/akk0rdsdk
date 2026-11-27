package org.akkord.lib

import android.content.Intent
import android.util.Log
import com.huawei.hms.iap.IapClient.PriceType
import com.huawei.hms.iap.entity.InAppPurchaseData.PurchaseState
import com.huawei.hms.iap.entity.OrderStatusCode

// Биллинг Huawei AppGallery на HMS IAP Kit (com.huawei.hms:iap). Нужен HMS Core на устройстве и вход в аккаунт Huawei.
// Приложению: зависимость libs.huawei.iap, agconnect-services.json в src/<флейвор>/assets (app id), активити
// HuaweiIapActivity в манифесте флейвора (см. HmsGateway.kt), товары - в AppGallery Connect.
// Товары: непотребляемые - restorePurchases сообщает о них при каждом вызове; потребляемые - пока не выданы
// (consumeProductItem); подписок нет. Вид товара HMS требует при покупке: его берём из запроса товаров (или
// спрашиваем HMS перед покупкой).
// При запуске окон нет: не вошёл в аккаунт Huawei - onBillingDisconnected, восстановления нет; вход в аккаунт
// предлагается только при покупке.
// Здесь только решения, вызовы HMS - в HmsGateway (HmsIapGateway); тесты - ../huawei_test/BillingImplementationTest.kt.
class BillingImplementation(
    val observer: BillingObserver,
    private val hms: HmsGateway = HmsIapGateway(),
) {
    companion object {
        const val PURCHASE_RESTORED = 0
        const val PURCHASE_BOUGHT   = 1
        private val ONE_TIME = listOf(PriceType.IN_APP_NONCONSUMABLE, PriceType.IN_APP_CONSUMABLE)
    }

    private val priceTypes = mutableMapOf<String, Int>()  // SKU -> вид товара в HMS

    fun init() {
        hms.checkEnvironment { code ->
            if (OrderStatusCode.ORDER_STATE_SUCCESS == code) {
                observer.onBillingSetupFinished()
            } else {
                log("isEnvReady: $code")
                observer.onBillingDisconnected()
            }
        }
    }

    // Непотребляемые и потребляемые по очереди; запрос успешен, только если ответили оба
    fun restorePurchases() {
        val owned = mutableListOf<HmsPurchase>()
        fun next(index: Int, success: Boolean) {
            if (index == ONE_TIME.size) {
                for (purchase in owned) report(purchase, PURCHASE_RESTORED)
                observer.onPurchasesQueryFinished(success)
                return
            }
            hms.ownedPurchases(ONE_TIME[index]) { code, purchases ->
                if (OrderStatusCode.ORDER_STATE_SUCCESS == code) owned += purchases else log("obtainOwnedPurchases: $code")
                next(index + 1, success && OrderStatusCode.ORDER_STATE_SUCCESS == code)
            }
        }
        next(0, true)
    }

    // Вошёл ли в аккаунт (иначе - окно входа), какого вида товар, шторка оплаты
    fun purchaseProdItem(productSKU: String) {
        hms.checkEnvironment { code ->
            when (code) {
                OrderStatusCode.ORDER_STATE_SUCCESS -> buy(productSKU)
                OrderStatusCode.ORDER_HWID_NOT_LOGIN -> hms.signIn { signedIn ->
                    if (OrderStatusCode.ORDER_STATE_SUCCESS == signedIn) buy(productSKU) else log("signIn: $signedIn")
                }
                else -> log("purchase $productSKU: isEnvReady $code")
            }
        }
    }

    fun purchaseSubscription(productSKU: String) {
    }

    fun consumeProductItem(purchaseToken: String, productSKU: String) {
        hms.consume(purchaseToken) { code ->
            if (OrderStatusCode.ORDER_STATE_SUCCESS == code) observer.onPurchaseConsumed(purchaseToken, productSKU)
            else log("consumeOwnedPurchase $productSKU: $code")
        }
    }

    fun onNewIntent(intent: Intent) {
    }

    // Товары обоих видов с ценами -> onProductDetails (пустой список - HMS не ответил); заодно запоминает вид товара
    fun queryProductDetails(prodList: Array<String?>?) {
        val skus = prodList.orEmpty().filterNotNull().filter { it.isNotEmpty() }
        productsOfBothTypes(skus) { products -> observer.onProductDetails(products) }
    }

    fun querySubscriptionProducts(prodList: Array<String>) {
    }

    // ---- решения ----

    private fun buy(productSKU: String) {
        val known = priceTypes[productSKU]
        if (known != null) {
            launch(productSKU, known)
            return
        }
        productsOfBothTypes(listOf(productSKU)) {
            val type = priceTypes[productSKU]
            if (type != null) launch(productSKU, type) else log("purchase $productSKU: no such product in AppGallery")
        }
    }

    private fun launch(productSKU: String, priceType: Int) {
        hms.purchase(productSKU, priceType) { code, purchase ->
            when (code) {
                OrderStatusCode.ORDER_STATE_SUCCESS -> purchase?.let { report(it, PURCHASE_BOUGHT) }
                // Куплено раньше (на другом устройстве): узнать из восстановления
                OrderStatusCode.ORDER_PRODUCT_OWNED -> restorePurchases()
                OrderStatusCode.ORDER_STATE_CANCEL -> {}
                else -> log("purchase $productSKU: $code")
            }
        }
    }

    private fun productsOfBothTypes(skus: List<String>, done: (List<ProductInfo>) -> Unit) {
        val found = mutableListOf<ProductInfo>()
        fun next(index: Int) {
            if (index == ONE_TIME.size) {
                done(found)
                return
            }
            val type = ONE_TIME[index]
            hms.products(type, skus) { code, products ->
                if (OrderStatusCode.ORDER_STATE_SUCCESS == code) {
                    for (p in products) priceTypes[p.productSKU] = type
                    found += products
                } else {
                    log("obtainProductInfo($type): $code")
                }
                next(index + 1)
            }
        }
        next(0)
    }

    // Только оплаченные: отложенная оплата (PENDING), отменённая и с возвратом денег игре не отдаются
    private fun report(purchase: HmsPurchase, boughtOrRestored: Int) {
        priceTypes[purchase.productSKU] = purchase.priceType
        if (PurchaseState.PURCHASED != purchase.state) {
            log("purchase ${purchase.productSKU}: state ${purchase.state}")
            return
        }
        observer.onPurchaseQueried(purchase.purchaseToken, purchase.productSKU, boughtOrRestored)
    }

    private fun log(message: String) {
        Log.d(Utils.TAG, "HMS IAP: $message")
    }
}
