package org.akkord.lib

import android.content.Intent
import android.util.Log
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

    fun restorePurchases() {
        // запрашиваем только те товары, покупка которых требует обработки
        getBillingClient().getPurchaseInteractor().getPurchases(
            productType = ProductType.NON_CONSUMABLE_PRODUCT,
            purchaseStatus = ProductPurchaseStatus.CONFIRMED,
        ).addOnSuccessListener { purchases ->
            handlePurchases(purchases)
        }

        getBillingClient().getPurchaseInteractor().getPurchases(
            productType = ProductType.CONSUMABLE_PRODUCT,
            purchaseStatus = ProductPurchaseStatus.PAID,
        ).addOnSuccessListener { purchases ->
            handlePurchases(purchases)
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
            observer.onPurchaseQueried(paymentResult.purchaseId.value, paymentResult.productId.value, 0)
        }
    }

    fun consumeProductItem(purchaseToken: String, productSKU: String) {
        Log.d(getTag(), "consumeProductItem: $purchaseToken, productSKU: $productSKU")

        getBillingClient().getPurchaseInteractor().getPurchase(PurchaseId(purchaseToken)).addOnSuccessListener { purchase ->
            when(purchase) {
                is ProductPurchase -> {
                    if (ProductPurchaseStatus.CONFIRMED == purchase.status) {
                        Log.d(getTag(), "consumeProductItem: $purchaseToken, productSKU: $productSKU already confirmed")
                        observer.onPurchaseConsumed(purchaseToken, productSKU)
                    }
                    else {
                        confirmPurchase(purchaseToken, productSKU, 1)
                    }
                }
                else -> {
                }
            }
        }
    }

    fun onNewIntent(intent: Intent) {
        RuStorePayClient.instance.getIntentInteractor().proceedIntent(intent)
    }

    fun queryProductDetails(prodList: Array<String?>?) {

    }

    private fun confirmPurchase(purchaseToken: String, productSKU: String, attemptNumber: Int) {
        Log.d(getTag(), "confirmPurchase: $purchaseToken, productSKU: $productSKU, attemptNumber: $attemptNumber")
        getBillingClient().getPurchaseInteractor().confirmTwoStepPurchase(PurchaseId(purchaseToken))
            .addOnSuccessListener {
                Log.d(getTag(), "confirmPurchase [OnSuccessListener]: $purchaseToken, productSKU: $productSKU")
                observer.onPurchaseConsumed(purchaseToken, productSKU)
            }
            .addOnFailureListener {
                Log.d(getTag(), "confirmPurchase [addOnFailureListener]: $purchaseToken, productSKU: $productSKU")
                if(attemptNumber < 5) {
                    confirmPurchase(purchaseToken, productSKU, attemptNumber + 1)
                }
            }
    }

    private fun handlePurchases(purchases: List<Purchase>) {
        for(purchase in purchases) {
            Log.d(getTag(), "purchase: $purchase")
            if(purchase is ProductPurchase) {
                handleProductPurchase(purchase)
            }
        }
    }

    private fun handleProductPurchase(productPurchase: ProductPurchase) {
        when (productPurchase.status) {
            // status PAID может быть только для CONSUMABLE товаров
            ProductPurchaseStatus.PAID -> {
                if (ProductType.CONSUMABLE_PRODUCT == productPurchase.productType) {
                    observer.onPurchaseQueried(productPurchase.purchaseId.value, productPurchase.productId.value, 0)
                }
            }

            // статус CONFIRMED может быть для любых товаров, но тут мы обрабатываем только NON_CONSUMABLE
            // CONFIRMED для CONSUMABLE мы обрабатываем в callback для confirmPurchase в consumeProductItem
            // тут мы его не обрабатываем, так как тут он может приходить нам многократно при восстановлении покупок, а в consumeProductItem - однократно
            ProductPurchaseStatus.CONFIRMED -> {
                if (ProductType.NON_CONSUMABLE_PRODUCT == productPurchase.productType) {
                    observer.onPurchaseQueried(productPurchase.purchaseId.value, productPurchase.productId.value, 0)
                }
            }

            else -> {

            }
        }
    }

    private fun getBillingClient() = RuStorePayClient.instance
    private fun getTag(): String = org.akkord.lib.Utils.TAG
}
