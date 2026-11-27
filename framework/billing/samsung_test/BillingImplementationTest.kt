package org.akkord.lib

import com.samsung.android.sdk.iap.lib.constants.HelperDefine
import org.junit.Assert.assertEquals
import org.junit.Test

// Логика биллинга Samsung (framework/billing/samsung/BillingImplementation.kt) против подделки Galaxy Store: что
// приложение узнаёт о покупках при запуске, восстановлении, покупке и выдаче; товары с ценами. Samsung IAP SDK и его
// окно оплаты здесь не участвуют (SamsungGateway) - их проверяет только покупка на устройстве.
// Запуск: ci/proj.android, gradlew :app:testSamsungDebugUnitTest. Лежит вне samsung: приложения подключают ту папку
// исходниками целиком.
class BillingImplementationTest {

    private val ok = HelperDefine.IAP_ERROR_NONE

    private class Events : BillingObserver {
        val log = mutableListOf<String>()
        override fun onBillingSetupFinished() { log += "setup" }
        override fun onBillingDisconnected() { log += "disconnected" }
        override fun onPurchaseQueried(purchaseToken: String?, productSKU: String?, type: Int) {
            log += (if (type == BillingImplementation.PURCHASE_BOUGHT) "bought" else "restored") + " $productSKU $purchaseToken"
        }
        override fun onPurchaseConsumed(purchaseToken: String?, productSKU: String?) { log += "consumed $productSKU $purchaseToken" }
        override fun onPurchasesQueryFinished(success: Boolean) { log += "restore finished $success" }
        override fun onProductDetails(products: List<ProductInfo>) {
            log += "products ${products.map { "${it.productSKU} ${it.priceMicros} ${it.currency}" }}"
        }
    }

    private inner class Store : SamsungGateway {
        var owned: Pair<Int, List<SamsungPurchase>> = ok to emptyList()
        var catalog: Pair<Int, List<ProductInfo>> = ok to emptyList()
        var paymentAnswer: Pair<Int, SamsungPurchase?> = ok to null
        var consumeCode = ok
        val calls = mutableListOf<String>()

        override fun ownedItems(done: (code: Int, purchases: List<SamsungPurchase>) -> Unit) { calls += "owned"; done(owned.first, owned.second) }
        override fun products(productSKUs: List<String>, done: (code: Int, products: List<ProductInfo>) -> Unit) {
            calls += "products $productSKUs"
            done(catalog.first, catalog.second.filter { it.productSKU in productSKUs })
        }
        override fun purchase(productSKU: String, done: (code: Int, purchase: SamsungPurchase?) -> Unit) {
            calls += "purchase $productSKU"
            done(paymentAnswer.first, paymentAnswer.second)
        }
        override fun consume(purchaseId: String, done: (code: Int) -> Unit) { calls += "consume $purchaseId"; done(consumeCode) }
    }

    private val events = Events()
    private val store = Store()
    private val billing = BillingImplementation(events, store)

    @Test
    fun the_billing_is_ready_at_once() {
        billing.init()
        assertEquals(listOf("setup"), events.log)
        assertEquals(emptyList<String>(), store.calls)
    }

    @Test
    fun restore_reports_every_owned_item() {
        store.owned = ok to listOf(SamsungPurchase("p1", "no_ads", false), SamsungPurchase("p2", "coins", true))
        billing.restorePurchases()
        assertEquals(listOf("restored no_ads p1", "restored coins p2", "restore finished true"), events.log)
    }

    @Test
    fun a_failed_restore_says_so_and_reports_nothing() {
        store.owned = HelperDefine.IAP_ERROR_NETWORK_NOT_AVAILABLE to emptyList()
        billing.restorePurchases()
        assertEquals(listOf("restore finished false"), events.log)
    }

    @Test
    fun a_paid_purchase_is_reported_as_bought() {
        store.paymentAnswer = ok to SamsungPurchase("p1", "paints_all", false)
        billing.purchaseProdItem("paints_all")
        assertEquals(listOf("purchase paints_all"), store.calls)
        assertEquals(listOf("bought paints_all p1"), events.log)
    }

    @Test
    fun a_cancelled_payment_gives_nothing() {
        store.paymentAnswer = HelperDefine.IAP_PAYMENT_IS_CANCELED to null
        billing.purchaseProdItem("paints_all")
        assertEquals(emptyList<String>(), events.log)
    }

    @Test
    fun an_item_already_purchased_restores_what_is_owned() {
        store.paymentAnswer = HelperDefine.IAP_ERROR_ALREADY_PURCHASED to null
        store.owned = ok to listOf(SamsungPurchase("p1", "no_ads", false))
        billing.purchaseProdItem("no_ads")
        assertEquals(listOf("purchase no_ads", "owned"), store.calls)
        assertEquals(listOf("restored no_ads p1", "restore finished true"), events.log)
    }

    @Test
    fun any_other_payment_error_gives_nothing() {
        store.paymentAnswer = HelperDefine.IAP_ERROR_PRODUCT_DOES_NOT_EXIST to null
        billing.purchaseProdItem("tanks_unknown")
        assertEquals(listOf("purchase tanks_unknown"), store.calls)
        assertEquals(emptyList<String>(), events.log)
    }

    @Test
    fun a_consumed_product_is_reported_after_galaxy_store_consumes_it() {
        billing.consumeProductItem("p2", "coins")
        assertEquals(listOf("consume p2"), store.calls)
        assertEquals(listOf("consumed coins p2"), events.log)
    }

    @Test
    fun a_failed_consumption_is_not_reported() {
        store.consumeCode = HelperDefine.IAP_ERROR_COMMON
        billing.consumeProductItem("p2", "coins")
        assertEquals(emptyList<String>(), events.log)
    }

    @Test
    fun product_details_come_with_prices() {
        store.catalog = ok to listOf(ProductInfo("no_ads", "₩5,900", 5_900_000_000L, "KRW", "Remove Ads"))
        billing.queryProductDetails(arrayOf("no_ads", null, ""))
        assertEquals(listOf("products [no_ads]"), store.calls)
        assertEquals(listOf("products [no_ads 5900000000 KRW]"), events.log)
    }

    @Test
    fun failed_product_details_come_as_an_empty_list() {
        store.catalog = HelperDefine.IAP_ERROR_NETWORK_NOT_AVAILABLE to listOf(ProductInfo("no_ads", "", 0, "", ""))
        billing.queryProductDetails(arrayOf("no_ads"))
        assertEquals(listOf("products []"), events.log)
    }
}
