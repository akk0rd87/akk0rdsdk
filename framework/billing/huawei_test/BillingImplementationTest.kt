package org.akkord.lib

import com.huawei.hms.iap.IapClient.PriceType
import com.huawei.hms.iap.entity.InAppPurchaseData.PurchaseState
import com.huawei.hms.iap.entity.OrderStatusCode
import org.junit.Assert.assertEquals
import org.junit.Test

// Логика биллинга Huawei (framework/billing/huawei/BillingImplementation.kt) против подделки HMS: что приложение
// узнаёт о покупках при запуске, восстановлении, покупке (со входом в аккаунт) и выдаче; товары с ценами. HMS IAP
// Kit, его шторка и HuaweiIapActivity здесь не участвуют (HmsGateway) - их проверяет только покупка на устройстве.
// Запуск: ci/proj.android, gradlew :app:testHuaweiStoreDebugUnitTest. Лежит вне huawei: приложения подключают ту папку
// исходниками целиком.
class BillingImplementationTest {

    private val ok = OrderStatusCode.ORDER_STATE_SUCCESS
    private val nonConsumable = PriceType.IN_APP_NONCONSUMABLE
    private val consumable = PriceType.IN_APP_CONSUMABLE

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

    // Отвечает сразу тем, что задал тест; запоминает, что у него просили
    private inner class Hms : HmsGateway {
        var environment = ok
        var signInCode = ok
        val owned = mutableMapOf<Int, Pair<Int, List<HmsPurchase>>>()     // вид товара -> ответ
        val catalog = mutableMapOf<Int, List<ProductInfo>>()             // вид товара -> товары этого вида
        var productsCode = ok
        var purchaseAnswer: Pair<Int, HmsPurchase?> = ok to null
        var consumeCode = ok
        val calls = mutableListOf<String>()

        override fun checkEnvironment(done: (code: Int) -> Unit) { calls += "isEnvReady"; done(environment) }
        override fun signIn(done: (code: Int) -> Unit) {
            calls += "signIn"
            if (signInCode == ok) environment = ok
            done(signInCode)
        }
        override fun ownedPurchases(priceType: Int, done: (code: Int, purchases: List<HmsPurchase>) -> Unit) {
            calls += "owned $priceType"
            val answer = owned[priceType] ?: (ok to emptyList())
            done(answer.first, answer.second)
        }
        override fun products(priceType: Int, productSKUs: List<String>, done: (code: Int, products: List<ProductInfo>) -> Unit) {
            calls += "products $priceType $productSKUs"
            done(productsCode, catalog[priceType].orEmpty().filter { it.productSKU in productSKUs })
        }
        override fun purchase(productSKU: String, priceType: Int, done: (code: Int, purchase: HmsPurchase?) -> Unit) {
            calls += "purchase $productSKU $priceType"
            done(purchaseAnswer.first, purchaseAnswer.second)
        }
        override fun consume(purchaseToken: String, done: (code: Int) -> Unit) { calls += "consume $purchaseToken"; done(consumeCode) }
    }

    private val events = Events()
    private val hms = Hms()
    private val billing = BillingImplementation(events, hms)

    private fun purchase(token: String, sku: String, type: Int = nonConsumable, state: Int = PurchaseState.PURCHASED) =
        HmsPurchase(token, sku, state, type)

    private fun product(sku: String, micros: Long = 1_990_000L) = ProductInfo(sku, "1,99 €", micros, "EUR", sku)

    // ---- запуск ----

    @Test
    fun the_billing_is_ready_when_the_user_is_signed_in() {
        billing.init()
        assertEquals(listOf("setup"), events.log)
    }

    @Test
    fun without_a_huawei_account_the_billing_is_off_and_no_window_shows() {
        hms.environment = OrderStatusCode.ORDER_HWID_NOT_LOGIN
        billing.init()
        assertEquals(listOf("disconnected"), events.log)
        assertEquals(listOf("isEnvReady"), hms.calls)
    }

    // ---- восстановление ----

    @Test
    fun restore_reports_paid_products_of_both_types_and_nothing_unpaid() {
        hms.owned[nonConsumable] = ok to listOf(
            purchase("t1", "no_ads"),
            purchase("t2", "paints_all", state = PurchaseState.REFUNDED),
            purchase("t3", "paints_bright", state = PurchaseState.PENDING),
        )
        hms.owned[consumable] = ok to listOf(purchase("t4", "coins", consumable))
        billing.restorePurchases()
        assertEquals(listOf("owned $nonConsumable", "owned $consumable"), hms.calls)
        assertEquals(listOf("restored no_ads t1", "restored coins t4", "restore finished true"), events.log)
    }

    @Test
    fun restore_with_one_type_failing_is_not_a_full_answer() {
        hms.owned[nonConsumable] = ok to listOf(purchase("t1", "no_ads"))
        hms.owned[consumable] = OrderStatusCode.ORDER_STATE_NET_ERROR to emptyList()
        billing.restorePurchases()
        assertEquals(listOf("restored no_ads t1", "restore finished false"), events.log)
    }

    // ---- покупка ----

    @Test
    fun a_purchase_asks_the_type_of_an_unknown_product_then_opens_the_sheet() {
        hms.catalog[nonConsumable] = listOf(product("no_ads"))
        hms.purchaseAnswer = ok to purchase("t1", "no_ads")
        billing.purchaseProdItem("no_ads")
        assertEquals(listOf("isEnvReady", "products $nonConsumable [no_ads]", "products $consumable [no_ads]",
            "purchase no_ads $nonConsumable"), hms.calls)
        assertEquals(listOf("bought no_ads t1"), events.log)
    }

    @Test
    fun a_product_known_from_the_product_details_is_bought_at_once() {
        hms.catalog[consumable] = listOf(product("coins"))
        billing.queryProductDetails(arrayOf("coins"))
        hms.calls.clear()
        hms.purchaseAnswer = ok to purchase("t1", "coins", consumable)
        billing.purchaseProdItem("coins")
        assertEquals(listOf("isEnvReady", "purchase coins $consumable"), hms.calls)
        assertEquals("bought coins t1", events.log.last())
    }

    @Test
    fun a_purchase_without_a_huawei_account_signs_in_first() {
        hms.environment = OrderStatusCode.ORDER_HWID_NOT_LOGIN
        hms.catalog[nonConsumable] = listOf(product("no_ads"))
        hms.purchaseAnswer = ok to purchase("t1", "no_ads")
        billing.purchaseProdItem("no_ads")
        assertEquals(listOf("isEnvReady", "signIn"), hms.calls.take(2))
        assertEquals(listOf("bought no_ads t1"), events.log)
    }

    @Test
    fun a_cancelled_sign_in_buys_nothing() {
        hms.environment = OrderStatusCode.ORDER_HWID_NOT_LOGIN
        hms.signInCode = OrderStatusCode.ORDER_STATE_CANCEL
        billing.purchaseProdItem("no_ads")
        assertEquals(listOf("isEnvReady", "signIn"), hms.calls)
        assertEquals(emptyList<String>(), events.log)
    }

    @Test
    fun a_product_unknown_to_appgallery_buys_nothing() {
        billing.purchaseProdItem("tanks_unknown")
        assertEquals(emptyList<String>(), events.log)
        assertEquals(false, hms.calls.any { it.startsWith("purchase") })
    }

    @Test
    fun a_pending_payment_gives_nothing() {
        hms.catalog[nonConsumable] = listOf(product("no_ads"))
        hms.purchaseAnswer = ok to purchase("t1", "no_ads", state = PurchaseState.PENDING)
        billing.purchaseProdItem("no_ads")
        assertEquals(emptyList<String>(), events.log)
    }

    @Test
    fun a_cancelled_sheet_gives_nothing() {
        hms.catalog[nonConsumable] = listOf(product("no_ads"))
        hms.purchaseAnswer = OrderStatusCode.ORDER_STATE_CANCEL to null
        billing.purchaseProdItem("no_ads")
        assertEquals(emptyList<String>(), events.log)
    }

    @Test
    fun a_product_already_owned_restores_what_is_owned() {
        hms.catalog[nonConsumable] = listOf(product("no_ads"))
        hms.purchaseAnswer = OrderStatusCode.ORDER_PRODUCT_OWNED to null
        hms.owned[nonConsumable] = ok to listOf(purchase("t1", "no_ads"))
        billing.purchaseProdItem("no_ads")
        assertEquals(listOf("restored no_ads t1", "restore finished true"), events.log)
    }

    // ---- выдача потребляемого товара ----

    @Test
    fun a_consumed_product_is_reported_after_hms_consumes_it() {
        billing.consumeProductItem("t1", "coins")
        assertEquals(listOf("consume t1"), hms.calls)
        assertEquals(listOf("consumed coins t1"), events.log)
    }

    @Test
    fun a_failed_consumption_is_not_reported() {
        hms.consumeCode = OrderStatusCode.ORDER_PRODUCT_NOT_OWNED
        billing.consumeProductItem("t1", "coins")
        assertEquals(emptyList<String>(), events.log)
    }

    // ---- товары с ценами ----

    @Test
    fun product_details_give_products_of_both_types_with_prices() {
        hms.catalog[nonConsumable] = listOf(product("no_ads", 1_990_000L))
        hms.catalog[consumable] = listOf(product("coins", 990_000L))
        billing.queryProductDetails(arrayOf("no_ads", null, "", "coins", "tanks_unknown"))
        assertEquals(listOf("products [no_ads 1990000 EUR, coins 990000 EUR]"), events.log)
    }

    @Test
    fun failed_product_details_come_as_an_empty_list() {
        hms.catalog[nonConsumable] = listOf(product("no_ads"))
        hms.productsCode = OrderStatusCode.ORDER_STATE_NET_ERROR
        billing.queryProductDetails(arrayOf("no_ads"))
        assertEquals(listOf("products []"), events.log)
    }
}
