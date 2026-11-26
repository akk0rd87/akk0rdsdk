package org.akkord.lib

import com.android.billingclient.api.BillingClient.BillingResponseCode
import com.android.billingclient.api.Purchase.PurchaseState
import org.junit.Assert.assertEquals
import org.junit.Test

// Логика биллинга Google Play (framework/billing/googleplay/BillingImplementation.java) против подделки Google Play:
// что приложение узнаёт о покупках при подключении, восстановлении, оплате и выдаче. Сама Play Billing Library и
// шторка оплаты здесь не участвуют (PlayGateway) - их проверяет только тестовая покупка на устройстве.
// Запуск: ci/proj.android, gradlew :app:testGooglePlayDebugUnitTest. Лежит вне googleplay: приложения подключают ту
// папку исходниками целиком.
class BillingImplementationTest {

    // ---- подделки ----

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
            log += "products ${products.map { "${it.productSKU} ${it.priceLabel} ${it.priceMicros} ${it.currency}" }}"
        }
    }

    // Отвечает сразу тем, что задал тест; запоминает, что у него просили. Сообщения Google Play (подключение,
    // итог шторки) тест шлёт сам через listener
    private class Play : PlayGateway {
        lateinit var listener: PlayGateway.Listener
        var owned: Pair<Int, List<PlayGateway.PlayPurchase>> = BillingResponseCode.OK to emptyList()
        var launchCode = BillingResponseCode.OK
        var acknowledgeCode = BillingResponseCode.OK
        var consumeCode = BillingResponseCode.OK
        var products: Pair<Int, List<ProductInfo>> = BillingResponseCode.OK to emptyList()
        val calls = mutableListOf<String>()

        override fun connect(listener: PlayGateway.Listener) { calls += "connect"; this.listener = listener }
        override fun queryPurchases(done: PlayGateway.PurchasesDone) { calls += "queryPurchases"; done.done(owned.first, owned.second) }
        override fun launchPurchase(productSKU: String, done: PlayGateway.Done) { calls += "launch $productSKU"; done.done(launchCode) }
        override fun acknowledge(purchaseToken: String, done: PlayGateway.Done) { calls += "acknowledge $purchaseToken"; done.done(acknowledgeCode) }
        override fun consume(purchaseToken: String, done: PlayGateway.Done) { calls += "consume $purchaseToken"; done.done(consumeCode) }
        override fun queryProducts(productSKUs: List<String>, done: PlayGateway.ProductsDone) {
            calls += "queryProducts $productSKUs"
            done.done(products.first, products.second)
        }
    }

    private val events = Events()
    private val play = Play()
    private val billing = BillingImplementation(events, play)

    private fun purchase(token: String, vararg skus: String, state: Int = PurchaseState.PURCHASED, acknowledged: Boolean = true) =
        PlayGateway.PlayPurchase(token, skus.toList(), state, acknowledged, "order-$token")

    // ---- подключение ----

    @Test
    fun the_billing_connects_at_once_and_reports_when_ready() {
        assertEquals(listOf("connect"), play.calls)
        play.listener.onSetupFinished(BillingResponseCode.OK)
        assertEquals(listOf("setup"), events.log)
    }

    @Test
    fun a_failed_setup_and_a_lost_connection_are_reported_as_disconnected() {
        play.listener.onSetupFinished(BillingResponseCode.BILLING_UNAVAILABLE)
        play.listener.onDisconnected()
        assertEquals(listOf("disconnected", "disconnected"), events.log)
    }

    // ---- восстановление ----

    @Test
    fun restore_reports_every_acknowledged_purchase_and_nothing_pending() {
        play.owned = BillingResponseCode.OK to listOf(
            purchase("t1", "no_ads"),
            purchase("t2", "paints_all"),
            purchase("t3", "paints_classic", state = PurchaseState.PENDING, acknowledged = false),
            purchase("t4", "paints_bright", state = PurchaseState.UNSPECIFIED_STATE),
        )
        billing.restorePurchases()
        assertEquals(listOf("restored no_ads t1", "restored paints_all t2", "restore finished true"), events.log)
        assertEquals(listOf("connect", "queryPurchases"), play.calls)
    }

    @Test
    fun restore_acknowledges_a_purchase_left_unacknowledged_and_gives_it_as_bought() {
        play.owned = BillingResponseCode.OK to listOf(purchase("t1", "no_ads", acknowledged = false))
        billing.restorePurchases()
        assertEquals(listOf("connect", "queryPurchases", "acknowledge t1"), play.calls)
        assertEquals(listOf("bought no_ads t1", "restore finished true"), events.log)
    }

    @Test
    fun a_purchase_is_given_even_if_its_acknowledgement_fails() {
        // Деньги списаны: товар игре отдаётся; неподтверждённую покупку Google вернёт через 3 дня, повтор - при
        // следующем восстановлении
        play.owned = BillingResponseCode.OK to listOf(purchase("t1", "no_ads", acknowledged = false))
        play.acknowledgeCode = BillingResponseCode.NETWORK_ERROR
        billing.restorePurchases()
        assertEquals(listOf("bought no_ads t1", "restore finished true"), events.log)
    }

    @Test
    fun one_purchase_of_several_products_reports_each_of_them() {
        play.owned = BillingResponseCode.OK to listOf(purchase("t1", "paints_classic", "", "paints_bright"))
        billing.restorePurchases()
        assertEquals(listOf("restored paints_classic t1", "restored paints_bright t1", "restore finished true"), events.log)
    }

    @Test
    fun a_failed_restore_says_so_and_reports_nothing() {
        play.owned = BillingResponseCode.SERVICE_UNAVAILABLE to emptyList()
        billing.restorePurchases()
        assertEquals(listOf("restore finished false"), events.log)
    }

    // ---- покупка ----

    @Test
    fun a_purchase_opens_the_sheet_and_the_paid_product_comes_acknowledged_as_bought() {
        billing.purchaseProdItem("paints_all")
        assertEquals(emptyList<String>(), events.log)  // шторка открыта, итога ещё нет
        play.listener.onPurchasesUpdated(BillingResponseCode.OK, listOf(purchase("t1", "paints_all", acknowledged = false)))
        assertEquals(listOf("connect", "launch paints_all", "acknowledge t1"), play.calls)
        assertEquals(listOf("bought paints_all t1"), events.log)
    }

    @Test
    fun a_purchase_that_did_not_start_gives_nothing() {
        play.launchCode = BillingResponseCode.ITEM_UNAVAILABLE
        billing.purchaseProdItem("tanks_unknown")
        assertEquals(emptyList<String>(), events.log)
    }

    @Test
    fun a_cancelled_sheet_gives_nothing() {
        play.listener.onPurchasesUpdated(BillingResponseCode.USER_CANCELED, emptyList())
        assertEquals(emptyList<String>(), events.log)
        assertEquals(listOf("connect"), play.calls)
    }

    @Test
    fun a_pending_payment_gives_nothing_until_it_is_paid() {
        play.listener.onPurchasesUpdated(BillingResponseCode.OK,
            listOf(purchase("t1", "paints_all", state = PurchaseState.PENDING, acknowledged = false)))
        assertEquals(emptyList<String>(), events.log)
        // Оплатили позже (наличные, перевод): Google Play сообщает той же покупкой
        play.listener.onPurchasesUpdated(BillingResponseCode.OK, listOf(purchase("t1", "paints_all", acknowledged = false)))
        assertEquals(listOf("bought paints_all t1"), events.log)
    }

    @Test
    fun an_item_already_owned_restores_what_is_owned() {
        play.owned = BillingResponseCode.OK to listOf(purchase("t1", "no_ads"))
        play.listener.onPurchasesUpdated(BillingResponseCode.ITEM_ALREADY_OWNED, emptyList())
        assertEquals(listOf("connect", "queryPurchases"), play.calls)
        assertEquals(listOf("restored no_ads t1", "restore finished true"), events.log)
    }

    // ---- выдача потребляемого товара ----

    @Test
    fun a_consumed_product_is_reported_after_google_play_consumes_it() {
        billing.consumeProductItem("t1", "coins")
        assertEquals(listOf("connect", "consume t1"), play.calls)
        assertEquals(listOf("consumed coins t1"), events.log)
    }

    @Test
    fun a_failed_consumption_is_not_reported() {
        play.consumeCode = BillingResponseCode.ITEM_NOT_OWNED
        billing.consumeProductItem("t1", "coins")
        assertEquals(emptyList<String>(), events.log)
    }

    // ---- товары с ценами ----

    @Test
    fun product_details_give_the_products_google_play_knows_with_prices() {
        play.products = BillingResponseCode.OK to listOf(
            ProductInfo("no_ads", "$4.99", 4_990_000L, "USD", "Remove Ads"),
            ProductInfo("paints_all", "$9.99", 9_990_000L, "USD", "All paints"),
        )
        billing.queryProductDetails(arrayOf("no_ads", null, "", "paints_all"))
        assertEquals(listOf("connect", "queryProducts [no_ads, paints_all]"), play.calls)
        assertEquals(listOf("products [no_ads $4.99 4990000 USD, paints_all $9.99 9990000 USD]"), events.log)
    }

    @Test
    fun failed_product_details_come_as_an_empty_list() {
        play.products = BillingResponseCode.SERVICE_UNAVAILABLE to listOf(ProductInfo("no_ads", "$4.99", 4_990_000L, "USD", ""))
        billing.queryProductDetails(arrayOf("no_ads"))
        assertEquals(listOf("products []"), events.log)
    }

    // ---- коды в логе ----

    @Test
    fun response_codes_have_names_for_the_log() {
        assertEquals("ITEM_ALREADY_OWNED", BillingImplementation.DecodeBillingResponse(BillingResponseCode.ITEM_ALREADY_OWNED))
        assertEquals("UNKNOWN(42)", BillingImplementation.DecodeBillingResponse(42))
    }
}
