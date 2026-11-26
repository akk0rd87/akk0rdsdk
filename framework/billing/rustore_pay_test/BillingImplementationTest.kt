package org.akkord.lib

import android.content.Intent
import org.junit.Assert.assertEquals
import org.junit.Test
import ru.rustore.sdk.pay.model.AcknowledgementState
import ru.rustore.sdk.pay.model.ProductPurchaseStatus
import ru.rustore.sdk.pay.model.ProductType
import ru.rustore.sdk.pay.model.PurchaseType
import ru.rustore.sdk.pay.model.SubscriptionPurchaseStatus

// Логика биллинга RuStore (framework/billing/rustore_pay/BillingImplementation.kt) против подделки стора: что
// приложение узнаёт о покупках при восстановлении, оплате и её сбоях. Сам Pay SDK и шторка оплаты здесь не участвуют
// (PayGateway) - их проверяет только тестовая покупка на устройстве.
// Запуск: ci/proj.android, gradlew :app:testRuStoreDebugUnitTest. Лежит вне rustore_pay: приложения подключают ту
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
        override fun onSubscriptionStatus(productSKU: String, active: Boolean, expirationMs: Long) {
            log += "subscription $productSKU active=$active until=$expirationMs"
        }
        override fun onSubscriptionProducts(products: List<SubscriptionProduct>) {
            log += "subscription products ${products.map { it.productSKU }}"
        }
        override fun onProductDetails(products: List<ProductInfo>) {
            log += "products ${products.map { "${it.productSKU} ${it.priceLabel} ${it.priceMicros} ${it.currency}" }}"
        }
    }

    // Отвечает сразу тем, что задал тест; запоминает, что у него просили
    private class Store : PayGateway {
        var purchases: Result<List<StorePurchase>> = Result.success(emptyList())
        val byId = mutableMapOf<String, StorePurchase>()       // getPurchase; нет в списке - ошибка сети
        var paid: PaymentResult? = null                         // ответ шторки: успех
        var failed: PaymentFailure? = null                      // или неудача
        var acknowledgeFails = false
        var products: Result<List<StoreProduct>> = Result.success(emptyList())
        val calls = mutableListOf<String>()

        override fun getPurchases(onSuccess: (List<StorePurchase>) -> Unit, onFailure: (Throwable) -> Unit) {
            calls += "getPurchases"
            purchases.fold(onSuccess, onFailure)
        }
        override fun getPurchase(purchaseId: String, onSuccess: (StorePurchase) -> Unit, onFailure: (Throwable) -> Unit) {
            calls += "getPurchase $purchaseId"
            byId[purchaseId]?.let(onSuccess) ?: onFailure(RuntimeException("no network"))
        }
        override fun purchase(productSKU: String, onSuccess: (PaymentResult) -> Unit, onFailure: (PaymentFailure) -> Unit) {
            calls += "purchase $productSKU"
            paid?.let(onSuccess) ?: onFailure(failed!!)
        }
        override fun acknowledge(purchaseId: String, onSuccess: () -> Unit, onFailure: (Throwable) -> Unit) {
            calls += "acknowledge $purchaseId"
            if (acknowledgeFails) onFailure(RuntimeException("no network")) else onSuccess()
        }
        override fun getProducts(productSKUs: List<String>, onSuccess: (List<StoreProduct>) -> Unit, onFailure: (Throwable) -> Unit) {
            calls += "getProducts $productSKUs"
            products.fold(onSuccess, onFailure)
        }
        override fun proceedIntent(intent: Intent) { calls += "proceedIntent" }
    }

    private val events = Events()
    private val store = Store()
    private val billing = BillingImplementation(events, store)

    // ---- покупки, какими их отдаёт стор ----

    private fun oneTime(
        id: String, sku: String, type: ProductType = ProductType.NON_CONSUMABLE_PRODUCT,
        status: ProductPurchaseStatus = ProductPurchaseStatus.CONFIRMED,
        ack: AcknowledgementState = AcknowledgementState.PENDING,
    ) = StorePurchase.OneTime(id, sku, type, status, ack)

    private fun subscription(id: String, sku: String, status: SubscriptionPurchaseStatus, until: Long) =
        StorePurchase.Subscription(id, sku, status, until)

    private fun paid(id: String, sku: String, type: ProductType = ProductType.NON_CONSUMABLE_PRODUCT,
                     stages: PurchaseType = PurchaseType.ONE_STEP) = PaymentResult(id, sku, stages, type, sandbox = false)

    private fun failed(id: String?, type: ProductType?, invalidActivePurchase: Boolean = false) =
        PaymentFailure(id, type, invalidActivePurchase, RuntimeException("sheet"))

    private fun tariff(sku: String) = SubscriptionProduct(sku, "199 ₽", 19900, "P1M", "")
    private fun info(sku: String) = ProductInfo(sku, "499 ₽", 499_000_000L, "RUB", sku.uppercase())

    // ---- старт и восстановление ----

    @Test
    fun init_reports_the_billing_ready_at_once() {
        billing.init()
        assertEquals(listOf("setup"), events.log)
    }

    @Test
    fun restore_reports_every_paid_non_consumable_and_nothing_unpaid() {
        store.purchases = Result.success(listOf(
            oneTime("p1", "no_ads"),
            oneTime("p2", "paints_all", ack = AcknowledgementState.ACKNOWLEDGED),  // для непотребляемых ничего не значит
            oneTime("p3", "paints_hold", status = ProductPurchaseStatus.PAID),     // холд двухстадийной оплаты
            oneTime("p4", "paints_refunded", status = ProductPurchaseStatus.REFUNDED),
            oneTime("p5", "paints_processing", status = ProductPurchaseStatus.PROCESSING),
        ))
        billing.restorePurchases()
        assertEquals(listOf("restored no_ads p1", "restored paints_all p2", "restore finished true"), events.log)
    }

    @Test
    fun restore_reports_a_consumable_only_until_it_is_consumed() {
        store.purchases = Result.success(listOf(
            oneTime("c1", "coins", ProductType.CONSUMABLE_PRODUCT),
            oneTime("c2", "coins", ProductType.CONSUMABLE_PRODUCT, ack = AcknowledgementState.ACKNOWLEDGED),
        ))
        billing.restorePurchases()
        assertEquals(listOf("restored coins c1", "restore finished true"), events.log)
    }

    @Test
    fun restore_reports_subscriptions_active_only_when_paid() {
        store.purchases = Result.success(listOf(
            subscription("s1", "premium", SubscriptionPurchaseStatus.ACTIVE, 1000),
            subscription("s2", "premium_year", SubscriptionPurchaseStatus.PAUSED, 2000),
        ))
        billing.restorePurchases()
        assertEquals(listOf(
            "subscription premium active=true until=1000",
            "subscription premium_year active=false until=2000",
            "restore finished true",
        ), events.log)
    }

    @Test
    fun a_failed_restore_says_so_and_reports_nothing() {
        store.purchases = Result.failure(RuntimeException("not signed in"))
        billing.restorePurchases()
        assertEquals(listOf("restore finished false"), events.log)
    }

    // ---- покупка ----

    @Test
    fun a_paid_purchase_is_reported_at_once_without_asking_the_store_again() {
        store.paid = paid("p1", "no_ads")
        billing.purchaseProdItem("no_ads")
        assertEquals(listOf("bought no_ads p1"), events.log)
        assertEquals(listOf("purchase no_ads"), store.calls)
    }

    @Test
    fun a_paid_consumable_is_reported_at_once_too() {
        store.paid = paid("c1", "coins", ProductType.CONSUMABLE_PRODUCT)
        billing.purchaseProdItem("coins")
        assertEquals(listOf("bought coins c1"), events.log)
    }

    @Test
    fun a_purchase_of_undefined_stages_is_checked_and_a_hold_is_not_reported() {
        store.paid = paid("p1", "paints_all", stages = PurchaseType.UNDEFINED)
        store.byId["p1"] = oneTime("p1", "paints_all", status = ProductPurchaseStatus.PAID)
        billing.purchaseProdItem("paints_all")
        assertEquals(listOf("purchase paints_all", "getPurchase p1"), store.calls)
        assertEquals(emptyList<String>(), events.log)
    }

    @Test
    fun a_two_stage_purchase_confirmed_by_the_store_is_reported() {
        store.paid = paid("p1", "paints_all", stages = PurchaseType.TWO_STEP)
        store.byId["p1"] = oneTime("p1", "paints_all")
        billing.purchaseProdItem("paints_all")
        assertEquals(listOf("bought paints_all p1"), events.log)
    }

    @Test
    fun a_bought_subscription_reports_its_status_from_the_purchase() {
        store.paid = paid("s1", "premium", ProductType.SUBSCRIPTION)
        store.byId["s1"] = subscription("s1", "premium", SubscriptionPurchaseStatus.ACTIVE, 5000)
        billing.purchaseSubscription("premium")
        assertEquals(listOf("subscription premium active=true until=5000"), events.log)
    }

    @Test
    fun the_sheet_closed_after_the_payment_still_gives_the_product() {
        store.failed = failed("p1", ProductType.NON_CONSUMABLE_PRODUCT)
        store.byId["p1"] = oneTime("p1", "paints_classic")
        billing.purchaseProdItem("paints_classic")
        assertEquals(listOf("bought paints_classic p1"), events.log)
    }

    @Test
    fun the_sheet_closed_before_the_payment_gives_nothing() {
        store.failed = failed("p1", ProductType.NON_CONSUMABLE_PRODUCT)
        store.byId["p1"] = oneTime("p1", "paints_classic", status = ProductPurchaseStatus.CANCELLED)
        billing.purchaseProdItem("paints_classic")
        assertEquals(emptyList<String>(), events.log)
    }

    @Test
    fun the_sheet_closed_and_the_check_failed_gives_nothing_until_the_next_restore() {
        store.failed = failed("p1", ProductType.NON_CONSUMABLE_PRODUCT)
        billing.purchaseProdItem("paints_classic")
        assertEquals(listOf("purchase paints_classic", "getPurchase p1"), store.calls)
        assertEquals(emptyList<String>(), events.log)
    }

    @Test
    fun a_non_consumable_that_failed_without_a_purchase_restores_what_is_owned() {
        // Так стор отвечает, например, на повторную покупку уже купленного: приложение узнаёт о нём из восстановления
        store.failed = failed(null, ProductType.NON_CONSUMABLE_PRODUCT)
        store.purchases = Result.success(listOf(oneTime("p1", "paints_all")))
        billing.purchaseProdItem("paints_all")
        assertEquals(listOf("purchase paints_all", "getPurchases"), store.calls)
        assertEquals(listOf("restored paints_all p1", "restore finished true"), events.log)
    }

    @Test
    fun an_invalid_active_purchase_restores_what_is_owned() {
        store.failed = failed(null, null, invalidActivePurchase = true)
        store.purchases = Result.success(listOf(oneTime("p1", "no_ads")))
        billing.purchaseProdItem("no_ads")
        assertEquals(listOf("restored no_ads p1", "restore finished true"), events.log)
    }

    @Test
    fun a_consumable_that_failed_without_a_purchase_restores_nothing() {
        store.failed = failed(null, ProductType.CONSUMABLE_PRODUCT)
        billing.purchaseProdItem("coins")
        assertEquals(listOf("purchase coins"), store.calls)
        assertEquals(emptyList<String>(), events.log)
    }

    @Test
    fun any_other_purchase_error_gives_nothing() {
        store.failed = failed(null, null)
        billing.purchaseProdItem("no_ads")
        assertEquals(listOf("purchase no_ads"), store.calls)
        assertEquals(emptyList<String>(), events.log)
    }

    // ---- выдача потребляемого товара ----

    @Test
    fun a_consumed_product_is_acknowledged_in_the_store() {
        billing.consumeProductItem("c1", "coins")
        assertEquals(listOf("acknowledge c1"), store.calls)
        assertEquals(listOf("consumed coins c1"), events.log)
    }

    @Test
    fun a_failed_acknowledgement_is_not_reported_as_consumed() {
        store.acknowledgeFails = true
        billing.consumeProductItem("c1", "coins")
        assertEquals(emptyList<String>(), events.log)
    }

    // ---- тарифы подписок ----

    @Test
    fun subscription_products_leave_out_one_time_products() {
        store.products = Result.success(listOf(
            StoreProduct(ProductType.NON_CONSUMABLE_PRODUCT, info("no_ads"), tariff("no_ads")),
            StoreProduct(ProductType.SUBSCRIPTION, info("premium"), tariff("premium")),
        ))
        billing.querySubscriptionProducts(arrayOf("no_ads", "premium"))
        assertEquals(listOf("subscription products [premium]"), events.log)
    }

    @Test
    fun failed_subscription_products_come_as_an_empty_list() {
        store.products = Result.failure(RuntimeException("no network"))
        billing.querySubscriptionProducts(arrayOf("premium"))
        assertEquals(listOf("subscription products []"), events.log)
    }

    // ---- разовые товары с ценами ----

    @Test
    fun product_details_give_one_time_products_with_prices_and_leave_out_subscriptions() {
        store.products = Result.success(listOf(
            StoreProduct(ProductType.NON_CONSUMABLE_PRODUCT, info("no_ads"), tariff("no_ads")),
            StoreProduct(ProductType.CONSUMABLE_PRODUCT, info("coins"), tariff("coins")),
            StoreProduct(ProductType.SUBSCRIPTION, info("premium"), tariff("premium")),
        ))
        billing.queryProductDetails(arrayOf("no_ads", null, "", "coins", "premium"))
        assertEquals(listOf("getProducts [no_ads, coins, premium]"), store.calls)
        assertEquals(listOf("products [no_ads 499 ₽ 499000000 RUB, coins 499 ₽ 499000000 RUB]"), events.log)
    }

    @Test
    fun failed_product_details_come_as_an_empty_list() {
        store.products = Result.failure(RuntimeException("no network"))
        billing.queryProductDetails(arrayOf("no_ads"))
        assertEquals(listOf("products []"), events.log)
    }

    // ---- возврат из приложения банка ----

    @Test
    fun a_new_intent_goes_to_the_store() {
        billing.onNewIntent(Intent())
        assertEquals(listOf("proceedIntent"), store.calls)
    }
}
