package org.akkord.lib

import android.content.Intent
import android.util.Log
import com.yandex.mobile.ads.impl.pu
import ru.rustore.sdk.pay.RuStorePayClient
import ru.rustore.sdk.pay.model.PreferredPurchaseType
import ru.rustore.sdk.pay.model.ProductId
import ru.rustore.sdk.pay.model.ProductPurchase
import ru.rustore.sdk.pay.model.ProductPurchaseParams
import ru.rustore.sdk.pay.model.ProductPurchaseResult
import ru.rustore.sdk.pay.model.ProductPurchaseStatus
import ru.rustore.sdk.pay.model.ProductType
import ru.rustore.sdk.pay.model.Purchase
import ru.rustore.sdk.pay.model.PurchaseId
import ru.rustore.sdk.pay.model.PurchaseType
import ru.rustore.sdk.pay.model.Quantity

class BillingImplementation(
    val observer: BillingObserver,
) {
    fun init() {
        // по умолчанию всегда считаем, что billing есть
        // возможно придется поменять логику, если мы будем использовать этот биллинг в версиях приложений,
        // скачанных вне RuStore
        observer.onBillingSetupFinished(0)
    }

    fun onNewIntent(intent: Intent) {
        RuStorePayClient.instance.getIntentInteractor().proceedIntent(intent)
    }

    fun queryProductDetails(prodList: Array<String?>?) {

    }

    fun restorePurchases() {
        // запрашиваем CONFIRMED, как уже подтвержденные покупки (для рекламы)
        getBillingClient().getPurchaseInteractor().getPurchases(
            productType = ProductType.NON_CONSUMABLE_PRODUCT,
            purchaseStatus = ProductPurchaseStatus.CONFIRMED,
        ).addOnSuccessListener { purchases ->
            handleRestoredPurchases(purchases)
        }

        // запрашиваем PAID, чтобы перевести в статус CONFIRMED
        getBillingClient().getPurchaseInteractor().getPurchases(
            purchaseStatus = ProductPurchaseStatus.PAID,
        ).addOnSuccessListener { purchases ->
            handleRestoredPurchases(purchases)
        }
    }

    fun purchaseProdItem(productSKU: String) {
        val params = ProductPurchaseParams(
            productId = ProductId(productSKU),
            quantity = Quantity(1),
        )

        getBillingClient().getPurchaseInteractor().purchase(
            params = params,
            preferredPurchaseType = PreferredPurchaseType.TWO_STEP,
        ).addOnSuccessListener { paymentResult: ProductPurchaseResult ->
            getBillingClient().getPurchaseInteractor().getPurchase(
                purchaseId = paymentResult.purchaseId,
            ).addOnSuccessListener { purchase ->
                handleCurrentPurchase(purchase)
            }
        }
    }

    private fun handleCurrentProductPurchase(productPurchase: ProductPurchase) {
        when (productPurchase.status) {
            // если покупка в статусе PAID, ее надо довести до статуса CONFIRMED
            ProductPurchaseStatus.PAID -> {
                confirmPurchase(
                    purchaseId = productPurchase.purchaseId,
                    productSKU = productPurchase.productId.value,
                    attemptNumber = 1,
                )
            }

            ProductPurchaseStatus.CONFIRMED -> {
                observer.onPurchaseQueried( // тут шлем callback для всех типов продуктов CONSUMABLE и NON-CONSUMABLE
                    purchaseToken = productPurchase.purchaseId.value,
                    productSKU = productPurchase.productId.value,
                    type = 0,
                )
            }

            else -> {

            }
        }
    }

    private fun handleCurrentPurchase(purchase: Purchase) {
        if(purchase is ProductPurchase) {
            handleCurrentProductPurchase(purchase)
        }
    }

    private fun confirmPurchase(purchaseId: PurchaseId, productSKU: String, attemptNumber: Int) {
        Log.d(getTag(), "confirmPurchase: $purchaseId, productSKU: $productSKU, attemptNumber: $attemptNumber")
        getBillingClient().getPurchaseInteractor().confirmTwoStepPurchase(purchaseId)
            .addOnSuccessListener {
                Log.d(getTag(), "confirmPurchase [OnSuccessListener]: $purchaseId, productSKU: $productSKU")
                observer.onPurchaseQueried(purchaseId.value, productSKU, 0)
            }
            .addOnFailureListener {
                Log.d(getTag(), "confirmPurchase [addOnFailureListener]: $purchaseId, productSKU: $productSKU")
                if(attemptNumber < 5) {
                    confirmPurchase(purchaseId, productSKU, attemptNumber + 1)
                }
            }
    }

    fun consumeProductItem(purchaseToken: String, productSKU: String) {
        Log.d(getTag(), "consumeProductItem: $purchaseToken, productSKU: $productSKU")
        // так как в Rustore нет как такового списания товара, то по умолчанию всегда возвращаем, что товар списан (фиктивно)
        observer.onPurchaseConsumed(purchaseToken, productSKU)
    }

    private fun handleRestoredPurchases(purchases: List<Purchase>) {
        for(purchase in purchases) {
            Log.d(getTag(), "purchase: $purchase")
            if(purchase is ProductPurchase) {
                handleRestoredProductPurchase(purchase)
            }
        }
    }

    private fun handleRestoredProductPurchase(productPurchase: ProductPurchase) {
        when (productPurchase.status) {
            // если покупка в статусе PAID, ее надо довести до статуса CONFIRMED
            ProductPurchaseStatus.PAID -> {
                confirmPurchase(
                    purchaseId = productPurchase.purchaseId,
                    productSKU = productPurchase.productId.value,
                    attemptNumber = 1,
                )
            }

            // здесь шлем callback только для NON-CONSUMABLE продуктов
            ProductPurchaseStatus.CONFIRMED -> {
                if( ProductType.NON_CONSUMABLE_PRODUCT == productPurchase.productType) {
                    observer.onPurchaseQueried(
                        purchaseToken = productPurchase.purchaseId.value,
                        productSKU = productPurchase.productId.value,
                        type = 0,
                    )
                }
            }

            else -> {

            }
        }
    }

    private fun getBillingClient() = RuStorePayClient.instance
    private fun getTag(): String = org.akkord.lib.Utils.TAG
}
