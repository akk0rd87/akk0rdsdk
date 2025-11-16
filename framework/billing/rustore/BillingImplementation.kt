package org.akkord.lib

import android.app.Activity
import android.content.Intent
import android.content.pm.PackageManager
import android.util.Log
import ru.rustore.sdk.billingclient.RuStoreBillingClient
import ru.rustore.sdk.billingclient.RuStoreBillingClientFactory
import ru.rustore.sdk.billingclient.model.product.ProductType
import ru.rustore.sdk.billingclient.model.purchase.PaymentResult
import ru.rustore.sdk.billingclient.model.purchase.Purchase
import ru.rustore.sdk.billingclient.model.purchase.PurchaseState

class BillingImplementation(
    val observer: BillingObserver,
) {
    private val billingClient: RuStoreBillingClient = getBillingManager()

    fun init() {
        // по умолчанию всегда считаем, что billing есть
        // возможно придется поменять логику, если мы будем использовать этот биллинг в версиях приложений,
        // скачанных вне RuStore
        observer.onBillingSetupFinished(0)
    }

    fun restorePurchases() {
        billingClient.purchases.getPurchases().addOnSuccessListener{ purchases: List<Purchase> ->
            for(purchase in purchases) {
                when(purchase.purchaseState) {
                    // status PAID может быть только для CONSUMABLE товаров
                    PurchaseState.PAID -> {
                        if(ProductType.CONSUMABLE == purchase.productType) {
                            observer.onPurchaseQueried(purchase.purchaseId, purchase.productId, 0)
                        }
                    }

                    // статус CONFIRMED может быть для любых товаров, но тут мы обрабатываем только NON_CONSUMABLE
                    // CONFIRMED для CONSUMABLE мы обрабатываем в callback для confirmPurchase в consumeProductItem
                    // тут мы его не обрабатываем, так как тут он может приходить нам многократно при восстановлении покупок, а в consumeProductItem - однократно
                    PurchaseState.CONFIRMED -> {
                        if(ProductType.NON_CONSUMABLE == purchase.productType) {
                            observer.onPurchaseQueried(purchase.purchaseId, purchase.productId, 0)
                        }
                    }
                    else -> { // do nothing

                    }
                }
            }
        }
    }

    fun purchaseProdItem(productSKU: String) {
        billingClient.purchases.purchaseProduct(productSKU).addOnSuccessListener { paymentResult ->
            if(paymentResult is PaymentResult.Success) {
                observer.onPurchaseQueried(paymentResult.purchaseId, paymentResult.productId, 0)
            }
        }
    }

    fun consumeProductItem(purchaseToken: String, productSKU: String) {
        billingClient.purchases.confirmPurchase(purchaseToken).addOnSuccessListener {
            observer.onPurchaseConsumed(purchaseToken, productSKU)
        }
    }

    fun onNewIntent(intent: Intent) {
        billingClient.onNewIntent(intent)
    }

    fun queryProductDetails(prodList: Array<String?>?) {

    }

    private fun getBillingManager(): RuStoreBillingClient {
        val activity = getContext()
        val bundle = activity.packageManager.getApplicationInfo(activity.packageName, PackageManager.GET_META_DATA).metaData

        // read data from AndroidManifest
        val consoleApplicationId = bundle.getInt("akk0rdsdk.rustore.app.id", 0)
        val deeplinkScheme = bundle.getString("akk0rdsdk.rustore.app.scheme", "yourappscheme")

        Log.d(getTag(), "onBilling consoleApplicationId = $consoleApplicationId")
        Log.d(getTag(), "onBilling deeplinkScheme = $deeplinkScheme")

        return RuStoreBillingClientFactory.create(
            context = activity,
            consoleApplicationId = consoleApplicationId.toString(),
            deeplinkScheme = deeplinkScheme,
        )
    }

    private fun getTag(): String = org.akkord.lib.Utils.TAG
    private fun getContext(): Activity = org.akkord.lib.Utils.GetContext()
}
