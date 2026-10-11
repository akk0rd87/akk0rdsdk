package org.akkord.lib

import android.app.Activity
import android.content.Intent
import android.content.res.Configuration
import android.util.Log
import ru.rustore.sdk.pay.RuStorePayClient
import ru.rustore.sdk.pay.model.AcknowledgementState
import ru.rustore.sdk.pay.model.MainPeriod
import ru.rustore.sdk.pay.model.Product
import ru.rustore.sdk.pay.model.ProductId
import ru.rustore.sdk.pay.model.ProductPurchase
import ru.rustore.sdk.pay.model.ProductPurchaseParams
import ru.rustore.sdk.pay.model.ProductPurchaseStatus
import ru.rustore.sdk.pay.model.ProductType
import ru.rustore.sdk.pay.model.Purchase
import ru.rustore.sdk.pay.model.PurchaseId
import ru.rustore.sdk.pay.model.RuStorePaymentException
import ru.rustore.sdk.pay.model.SdkTheme
import ru.rustore.sdk.pay.model.SubscriptionPurchase
import ru.rustore.sdk.pay.model.SubscriptionPurchaseStatus
import ru.rustore.sdk.pay.model.TrialPeriod

// Биллинг RuStore на Pay SDK (ru.rustore.sdk:pay), пришёл на смену billingclient (framework/billing/rustore).
// SDK инициализируется сам, приложению нужно прописать в AndroidManifest.xml:
//   <meta-data android:name="console_app_id_value" android:value="..."/>   id приложения из RuStore Консоли
//   <meta-data android:name="sdk_pay_scheme_value" android:value="..."/>   схема диплинка для возврата из приложения банка
// и intent-filter с этой схемой у Activity, которая передаёт intent в onNewIntent.
class BillingImplementation(
    val observer: BillingObserver,
) {
    private val PURCHASE_RESTORED = 0
    private val PURCHASE_BOUGHT   = 1

    fun init() {
        // Отдельного подключения к сервису у Pay SDK нет
        observer.onBillingSetupFinished()
    }

    fun restorePurchases() {
        try {
            // Без фильтров возвращаются покупки в статусах PAID, CONFIRMED, ACTIVE и PAUSED
            payClient().getPurchaseInteractor().getPurchases()
                .addOnSuccessListener { purchases: List<Purchase> ->
                    Log.d(getTag(), "RuStore pay: getPurchases: ${purchases.size}")
                    for (purchase in purchases) {
                        reportPurchase(purchase, PURCHASE_RESTORED)
                    }
                    observer.onPurchasesQueryFinished(true)
                }
                .addOnFailureListener { e ->
                    Log.d(getTag(), "RuStore pay: getPurchases failed: $e")
                    observer.onPurchasesQueryFinished(false)
                }
        }
        catch(e: Exception) {
            Utils.handleException(e, "restorePurchases")
            observer.onPurchasesQueryFinished(false)
        }
    }

    fun purchaseProdItem(productSKU: String) {
        purchase(productSKU)
    }

    // Подписки оплачиваются тем же методом, что и разовые товары (всегда в одну стадию)
    fun purchaseSubscription(productSKU: String) {
        purchase(productSKU)
    }

    // Отмечает потребляемый товар выданным: restorePurchases больше не вернёт эту покупку
    fun consumeProductItem(purchaseToken: String, productSKU: String) {
        payClient().getPurchaseInteractor()
            .updateAcknowledgementState(PurchaseId(purchaseToken), AcknowledgementState.ACKNOWLEDGED)
            .addOnSuccessListener {
                observer.onPurchaseConsumed(purchaseToken, productSKU)
            }
            .addOnFailureListener { e ->
                Log.d(getTag(), "RuStore pay: updateAcknowledgementState failed: $e")
            }
    }

    fun onNewIntent(intent: Intent) {
        // Возврат из приложения банка: SDK восстанавливает платёжную шторку
        payClient().getIntentInteractor().proceedIntent(intent, getSdkTheme())
    }

    fun queryProductDetails(prodList: Array<String?>?) {

    }

    // Работает без авторизации пользователя и без установленного RuStore
    fun querySubscriptionProducts(prodList: Array<String>) {
        try {
            payClient().getProductInteractor().getProducts(prodList.map { ProductId(it) })
                .addOnSuccessListener { products: List<Product> ->
                    Log.d(getTag(), "RuStore pay: getProducts: ${products.map { it.productId.value }}")
                    observer.onSubscriptionProducts(
                        products.filter { it.type == ProductType.SUBSCRIPTION }.map { toSubscriptionProduct(it) }
                    )
                }
                .addOnFailureListener { e ->
                    Log.d(getTag(), "RuStore pay: getProducts failed: $e")
                    observer.onSubscriptionProducts(emptyList())
                }
        }
        catch(e: Exception) {
            Utils.handleException(e, "querySubscriptionProducts")
            observer.onSubscriptionProducts(emptyList())
        }
    }

    private fun purchase(productSKU: String) {
        payClient().getPurchaseInteractor()
            .purchase(params = ProductPurchaseParams(ProductId(productSKU)), sdkTheme = getSdkTheme())
            .addOnSuccessListener { result ->
                queryPurchase(result.purchaseId)
            }
            .addOnFailureListener { e ->
                Log.d(getTag(), "RuStore pay: purchase failed: $e")
                // Шторку закрыли до получения результата: состояние покупки неизвестно, перепроверяем
                if (e is RuStorePaymentException.ProductPurchaseCancelled) {
                    e.purchaseId?.let { queryPurchase(it) }
                }
            }
    }

    private fun queryPurchase(purchaseId: PurchaseId) {
        payClient().getPurchaseInteractor().getPurchase(purchaseId)
            .addOnSuccessListener { purchase: Purchase ->
                reportPurchase(purchase, PURCHASE_BOUGHT)
            }
            .addOnFailureListener { e ->
                Log.d(getTag(), "RuStore pay: getPurchase failed: $e")
            }
    }

    private fun reportPurchase(purchase: Purchase, boughtOrRestored: Int) {
        Log.d(getTag(), "RuStore pay: purchase: $purchase")
        when (purchase) {
            is ProductPurchase -> {
                // PAID (холд двухстадийной оплаты) не обрабатываем: покупаем только в одну стадию
                if (purchase.status != ProductPurchaseStatus.CONFIRMED) {
                    return
                }
                // CONFIRMED у потребляемого товара остаётся навсегда, поэтому сообщаем только о ещё не выданных
                // (см. consumeProductItem)
                val delivered = ProductType.CONSUMABLE_PRODUCT == purchase.productType
                    && AcknowledgementState.PENDING != purchase.acknowledgementState
                if (!delivered) {
                    observer.onPurchaseQueried(purchase.purchaseId.value, purchase.productId.value, boughtOrRestored)
                }
            }

            is SubscriptionPurchase -> {
                // Доступ даёт только ACTIVE: PAUSED — холд-период, списание не прошло
                observer.onSubscriptionStatus(
                    purchase.productId.value,
                    SubscriptionPurchaseStatus.ACTIVE == purchase.status,
                    purchase.expirationDate.time,
                )
            }
        }
    }

    private fun toSubscriptionProduct(product: Product): SubscriptionProduct {
        val periods = product.subscriptionInfo?.periods.orEmpty()
        val mainPeriod = periods.filterIsInstance<MainPeriod>().firstOrNull()
        val trialPeriod = periods.filterIsInstance<TrialPeriod>().firstOrNull()

        Log.d(getTag(), "RuStore pay: product: ${product.productId.value} ${product.amountLabel.value} main=${mainPeriod?.duration} trial=${trialPeriod?.duration}")
        return SubscriptionProduct(
            productSKU = product.productId.value,
            priceLabel = product.amountLabel.value,
            priceMinor = (mainPeriod?.price ?: product.price?.value ?: 0).toLong(),
            period = mainPeriod?.duration.orEmpty(),
            trialPeriod = trialPeriod?.duration.orEmpty(),
        )
    }

    private fun getSdkTheme(): SdkTheme {
        val nightMode = getContext().resources.configuration.uiMode and Configuration.UI_MODE_NIGHT_MASK
        return if (Configuration.UI_MODE_NIGHT_YES == nightMode) SdkTheme.DARK else SdkTheme.LIGHT
    }

    private fun payClient(): RuStorePayClient = RuStorePayClient.instance
    private fun getTag(): String = org.akkord.lib.Utils.TAG
    private fun getContext(): Activity = org.akkord.lib.Utils.GetContext()
}
