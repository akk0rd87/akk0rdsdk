package org.akkord.lib

import android.app.Activity
import android.content.Intent
import android.content.res.Configuration
import ru.rustore.sdk.pay.RuStorePayClient
import ru.rustore.sdk.pay.model.AcknowledgementState
import ru.rustore.sdk.pay.model.MainPeriod
import ru.rustore.sdk.pay.model.PreferredPurchaseType
import ru.rustore.sdk.pay.model.Product
import ru.rustore.sdk.pay.model.ProductId
import ru.rustore.sdk.pay.model.ProductPurchase
import ru.rustore.sdk.pay.model.ProductPurchaseParams
import ru.rustore.sdk.pay.model.ProductPurchaseStatus
import ru.rustore.sdk.pay.model.ProductType
import ru.rustore.sdk.pay.model.Purchase
import ru.rustore.sdk.pay.model.PurchaseId
import ru.rustore.sdk.pay.model.PurchaseType
import ru.rustore.sdk.pay.model.RuStorePaymentException
import ru.rustore.sdk.pay.model.SdkTheme
import ru.rustore.sdk.pay.model.SubscriptionPurchase
import ru.rustore.sdk.pay.model.SubscriptionPurchaseStatus
import ru.rustore.sdk.pay.model.TrialPeriod

// Всё, что BillingImplementation просит у RuStore Pay SDK, - без решений: только вызовы и перекладка ответов в
// простые классы ниже. Отдельно, чтобы логику покупок проверяли юнит-тесты (../rustore_pay_test/BillingImplementationTest.kt) с
// подделкой вместо стора: у RuStorePayClient конструкторы с внутренними типами SDK, а у моделей SDK (покупки, ответы,
// исключения) конструкторы internal. Статусы и типы - перечисления самого SDK: что из них что значит, решает
// BillingImplementation. Каждый вызов кончается ровно одним из onSuccess / onFailure (исключение при вызове - тоже
// onFailure).
interface PayGateway {
    // Без фильтров: покупки в статусах PAID, CONFIRMED, ACTIVE и PAUSED
    fun getPurchases(onSuccess: (List<StorePurchase>) -> Unit, onFailure: (Throwable) -> Unit)
    fun getPurchase(purchaseId: String, onSuccess: (StorePurchase) -> Unit, onFailure: (Throwable) -> Unit)
    // Оплата в одну стадию: все способы оплаты её поддерживают (двухстадийную СБП не умеет), подписки - только её
    fun purchase(productSKU: String, onSuccess: (PaymentResult) -> Unit, onFailure: (PaymentFailure) -> Unit)
    fun acknowledge(purchaseId: String, onSuccess: () -> Unit, onFailure: (Throwable) -> Unit)
    fun getProducts(productSKUs: List<String>, onSuccess: (List<StoreProduct>) -> Unit, onFailure: (Throwable) -> Unit)
    fun proceedIntent(intent: Intent)
}

// Покупка, какой её знает стор (getPurchases, getPurchase)
sealed class StorePurchase {
    data class OneTime(
        val purchaseId: String,
        val productSKU: String,
        val type: ProductType,
        val status: ProductPurchaseStatus,
        val acknowledgement: AcknowledgementState,
    ) : StorePurchase()

    data class Subscription(
        val purchaseId: String,
        val productSKU: String,
        val status: SubscriptionPurchaseStatus,
        val expirationMs: Long,
    ) : StorePurchase()
}

// Шторка оплаты закрылась успешно
data class PaymentResult(
    val purchaseId: String,
    val productSKU: String,
    val purchaseType: PurchaseType,
    val productType: ProductType,
    val sandbox: Boolean,
)

// Шторка оплаты закрылась без успеха. purchaseId - если покупка успела появиться (её состояние надо спросить у стора),
// productType - если стор его сообщил, invalidActivePurchase - RuStorePayInvalidActivePurchase
data class PaymentFailure(
    val purchaseId: String?,
    val productType: ProductType?,
    val invalidActivePurchase: Boolean,
    val cause: Throwable,
)

// Товар из getProducts: info - как показать разовый товар, subscription - тариф подписки (оба в независимом от стора
// виде; у разового товара в subscription только SKU и цена)
data class StoreProduct(val type: ProductType, val info: ProductInfo, val subscription: SubscriptionProduct)

class RuStorePayGateway : PayGateway {
    override fun getPurchases(onSuccess: (List<StorePurchase>) -> Unit, onFailure: (Throwable) -> Unit) = call("getPurchases", onFailure) {
        purchases().getPurchases()
            .addOnSuccessListener { list -> onSuccess(list.mapNotNull { toStorePurchase(it) }) }
            .addOnFailureListener { onFailure(it) }
    }

    override fun getPurchase(purchaseId: String, onSuccess: (StorePurchase) -> Unit, onFailure: (Throwable) -> Unit) = call("getPurchase", onFailure) {
        purchases().getPurchase(PurchaseId(purchaseId))
            .addOnSuccessListener { purchase ->
                toStorePurchase(purchase)?.let(onSuccess) ?: onFailure(IllegalStateException("unknown purchase $purchase"))
            }
            .addOnFailureListener { onFailure(it) }
    }

    // Вызывается из потока игры (JNI), а шторка оплаты - это Activity: запускаем с UI-потока
    override fun purchase(productSKU: String, onSuccess: (PaymentResult) -> Unit, onFailure: (PaymentFailure) -> Unit) {
        val fail = { e: Throwable -> onFailure(toPaymentFailure(e)) }
        try {
            getContext().runOnUiThread {
                call("purchase", fail) {
                    purchases()
                        .purchase(
                            params = ProductPurchaseParams(ProductId(productSKU)),
                            preferredPurchaseType = PreferredPurchaseType.ONE_STEP,
                            sdkTheme = getSdkTheme(),
                        )
                        .addOnSuccessListener { r ->
                            onSuccess(PaymentResult(r.purchaseId.value, r.productId.value, r.purchaseType, r.productType, r.sandbox))
                        }
                        .addOnFailureListener(fail)
                }
            }
        }
        catch(e: Exception) {
            Utils.handleException(e, "purchase")
            fail(e)
        }
    }

    override fun acknowledge(purchaseId: String, onSuccess: () -> Unit, onFailure: (Throwable) -> Unit) = call("acknowledge", onFailure) {
        purchases().updateAcknowledgementState(PurchaseId(purchaseId), AcknowledgementState.ACKNOWLEDGED)
            .addOnSuccessListener { onSuccess() }
            .addOnFailureListener { onFailure(it) }
    }

    // Работает без авторизации пользователя и без установленного RuStore
    override fun getProducts(productSKUs: List<String>, onSuccess: (List<StoreProduct>) -> Unit, onFailure: (Throwable) -> Unit) = call("getProducts", onFailure) {
        RuStorePayClient.instance.getProductInteractor().getProducts(productSKUs.map { ProductId(it) })
            .addOnSuccessListener { list -> onSuccess(list.map { StoreProduct(it.type, toProductInfo(it), toSubscriptionProduct(it)) }) }
            .addOnFailureListener { onFailure(it) }
    }

    // Возврат из приложения банка: SDK восстанавливает платёжную шторку
    override fun proceedIntent(intent: Intent) {
        try {
            RuStorePayClient.instance.getIntentInteractor().proceedIntent(intent, getSdkTheme())
        }
        catch(e: Exception) {
            Utils.handleException(e, "proceedIntent")
        }
    }

    private fun toStorePurchase(purchase: Purchase): StorePurchase? = when (purchase) {
        is ProductPurchase -> StorePurchase.OneTime(
            purchase.purchaseId.value, purchase.productId.value, purchase.productType, purchase.status,
            purchase.acknowledgementState,
        )
        is SubscriptionPurchase -> StorePurchase.Subscription(
            purchase.purchaseId.value, purchase.productId.value, purchase.status, purchase.expirationDate.time,
        )
        else -> null
    }

    private fun toPaymentFailure(e: Throwable): PaymentFailure = when (e) {
        is RuStorePaymentException.ProductPurchaseCancelled -> PaymentFailure(e.purchaseId?.value, e.productType, false, e)
        is RuStorePaymentException.ProductPurchaseException -> PaymentFailure(e.purchaseId?.value, e.productType, false, e)
        is RuStorePaymentException.RuStorePayInvalidActivePurchase -> PaymentFailure(null, null, true, e)
        else -> PaymentFailure(null, null, false, e)
    }

    // Цена в RuStore - в копейках (минимальных единицах валюты), в BillingObserver - в миллионных долях: x 10 000
    private fun toProductInfo(product: Product) = ProductInfo(
        productSKU = product.productId.value,
        priceLabel = product.amountLabel.value,
        priceMicros = (product.price?.value ?: 0).toLong() * 10_000L,
        currency = product.currency?.value.orEmpty(),
        title = product.title?.value.orEmpty(),
    )

    private fun toSubscriptionProduct(product: Product): SubscriptionProduct {
        val periods = product.subscriptionInfo?.periods.orEmpty()
        val mainPeriod = periods.filterIsInstance<MainPeriod>().firstOrNull()
        val trialPeriod = periods.filterIsInstance<TrialPeriod>().firstOrNull()
        return SubscriptionProduct(
            productSKU = product.productId.value,
            priceLabel = product.amountLabel.value,
            priceMinor = (mainPeriod?.price ?: product.price?.value ?: 0).toLong(),
            period = mainPeriod?.duration.orEmpty(),
            trialPeriod = trialPeriod?.duration.orEmpty(),
        )
    }

    private inline fun call(what: String, noinline onFailure: (Throwable) -> Unit, action: () -> Unit) {
        try {
            action()
        }
        catch(e: Exception) {
            Utils.handleException(e, what)
            onFailure(e)
        }
    }

    private fun purchases() = RuStorePayClient.instance.getPurchaseInteractor()

    private fun getSdkTheme(): SdkTheme {
        val nightMode = getContext().resources.configuration.uiMode and Configuration.UI_MODE_NIGHT_MASK
        return if (Configuration.UI_MODE_NIGHT_YES == nightMode) SdkTheme.DARK else SdkTheme.LIGHT
    }

    private fun getContext(): Activity = Utils.GetContext()
}
