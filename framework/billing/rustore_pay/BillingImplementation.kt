package org.akkord.lib

import android.content.Intent
import android.util.Log
import ru.rustore.sdk.pay.model.AcknowledgementState
import ru.rustore.sdk.pay.model.ProductPurchaseStatus
import ru.rustore.sdk.pay.model.ProductType
import ru.rustore.sdk.pay.model.PurchaseType
import ru.rustore.sdk.pay.model.SubscriptionPurchaseStatus

// Биллинг RuStore на Pay SDK (ru.rustore.sdk:pay), пришёл на смену billingclient (framework/billing/rustore).
// SDK инициализируется сам, приложению нужно прописать в AndroidManifest.xml:
//   <meta-data android:name="console_app_id_value" android:value="..."/>   id приложения из RuStore Консоли
//   <meta-data android:name="sdk_pay_scheme_value" android:value="..."/>   схема диплинка для возврата из приложения банка
// и intent-filter с этой схемой у Activity, которая передаёт intent в BillingManager.onNewIntent (и из onCreate при
// savedInstanceState == null), launchMode singleTop или singleTask.
// Товары: непотребляемые (NON_CONSUMABLE_PRODUCT) - restorePurchases сообщает о них при каждом вызове, как Google Play,
// подтверждать (acknowledge) их не нужно, по документации RuStore это ни на что не влияет; потребляемые - до
// consumeProductItem; подписки - через onSubscriptionStatus.
// Здесь только решения, вызовы стора - в PayGateway; тесты - ../rustore_pay_test/BillingImplementationTest.kt.
class BillingImplementation(
    val observer: BillingObserver,
    private val pay: PayGateway = RuStorePayGateway(),
) {
    companion object {
        const val PURCHASE_RESTORED = 0
        const val PURCHASE_BOUGHT   = 1
    }

    fun init() {
        // Отдельного подключения к сервису у Pay SDK нет
        observer.onBillingSetupFinished()
    }

    fun restorePurchases() {
        pay.getPurchases(
            { purchases ->
                log("getPurchases: ${purchases.size}")
                for (purchase in purchases) {
                    reportPurchase(purchase, PURCHASE_RESTORED)
                }
                observer.onPurchasesQueryFinished(true)
            },
            { e ->
                log("getPurchases failed: $e")
                observer.onPurchasesQueryFinished(false)
            },
        )
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
        pay.acknowledge(
            purchaseToken,
            { observer.onPurchaseConsumed(purchaseToken, productSKU) },
            { e -> log("updateAcknowledgementState failed: $e") },
        )
    }

    fun onNewIntent(intent: Intent) {
        pay.proceedIntent(intent)
    }

    // Разовые товары с ценами для показа в игре -> onProductDetails (пустой список - стор не ответил).
    // Работает без авторизации пользователя и без установленного RuStore
    fun queryProductDetails(prodList: Array<String?>?) {
        pay.getProducts(
            prodList.orEmpty().filterNotNull().filter { it.isNotEmpty() },
            { products ->
                log("getProducts: ${products.map { it.info.productSKU }}")
                observer.onProductDetails(products.filter { it.type != ProductType.SUBSCRIPTION }.map { it.info })
            },
            { e ->
                log("getProducts failed: $e")
                observer.onProductDetails(emptyList())
            },
        )
    }

    fun querySubscriptionProducts(prodList: Array<String>) {
        pay.getProducts(
            prodList.toList(),
            { products ->
                log("getProducts: ${products.map { it.subscription.productSKU }}")
                observer.onSubscriptionProducts(products.filter { it.type == ProductType.SUBSCRIPTION }.map { it.subscription })
            },
            { e ->
                log("getProducts failed: $e")
                observer.onSubscriptionProducts(emptyList())
            },
        )
    }

    // Одна стадия: деньги списываются сразу, покупка становится CONFIRMED (двухстадийную PAID не обрабатываем)
    private fun purchase(productSKU: String) {
        pay.purchase(productSKU, { result -> onPurchaseResult(result) }, { e -> onPurchaseFailed(productSKU, e) })
    }

    private fun onPurchaseResult(result: PaymentResult) {
        log("purchase result: $result")
        // Успешная оплата в одну стадию - это CONFIRMED: товар выдаём сразу, без второго запроса в сеть
        // (если он не пройдёт, покупка вернётся только со следующим restorePurchases).
        // Подписку и двухстадийную оплату перепроверяем: статус подписки и холд узнаются только из покупки
        if (PurchaseType.ONE_STEP == result.purchaseType && ProductType.SUBSCRIPTION != result.productType) {
            observer.onPurchaseQueried(result.purchaseId, result.productSKU, PURCHASE_BOUGHT)
        } else {
            queryPurchase(result.purchaseId)
        }
    }

    private fun onPurchaseFailed(productSKU: String, failure: PaymentFailure) {
        log("purchase $productSKU failed: ${failure.cause}")
        if (failure.purchaseId != null) {
            // Шторку закрыли до получения результата или оплата сломалась на полпути: состояние покупки неизвестно,
            // спрашиваем у сервера, о CONFIRMED сообщит reportPurchase
            queryPurchase(failure.purchaseId)
        } else if (ProductType.NON_CONSUMABLE_PRODUCT == failure.productType || failure.invalidActivePurchase) {
            // Непотребляемый товар, скорее всего, уже куплен (как ITEM_ALREADY_OWNED в Google Play):
            // восстанавливаем покупки, чтобы приложение узнало о нём
            restorePurchases()
        }
    }

    private fun queryPurchase(purchaseId: String) {
        pay.getPurchase(
            purchaseId,
            { purchase -> reportPurchase(purchase, PURCHASE_BOUGHT) },
            { e -> log("getPurchase failed: $e") },
        )
    }

    private fun reportPurchase(purchase: StorePurchase, boughtOrRestored: Int) {
        log("purchase: $purchase")
        when (purchase) {
            is StorePurchase.OneTime -> {
                // PAID (холд двухстадийной оплаты) не обрабатываем: покупаем только в одну стадию
                if (purchase.status != ProductPurchaseStatus.CONFIRMED) {
                    return
                }
                // CONFIRMED у потребляемого товара остаётся навсегда, поэтому сообщаем только о ещё не выданных
                // (см. consumeProductItem)
                val delivered = ProductType.CONSUMABLE_PRODUCT == purchase.type
                    && AcknowledgementState.PENDING != purchase.acknowledgement
                if (!delivered) {
                    observer.onPurchaseQueried(purchase.purchaseId, purchase.productSKU, boughtOrRestored)
                }
            }

            is StorePurchase.Subscription -> {
                // Доступ даёт только ACTIVE: PAUSED — холд-период, списание не прошло
                observer.onSubscriptionStatus(
                    purchase.productSKU,
                    SubscriptionPurchaseStatus.ACTIVE == purchase.status,
                    purchase.expirationMs,
                )
            }
        }
    }

    private fun log(message: String) {
        Log.d(Utils.TAG, "RuStore pay: $message")
    }
}
