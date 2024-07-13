package org.akkord.lib

import android.app.Activity
import android.content.Intent
import android.content.pm.PackageManager
import android.util.Log
import ru.rustore.sdk.billingclient.RuStoreBillingClient
import ru.rustore.sdk.billingclient.RuStoreBillingClientFactory
import ru.rustore.sdk.billingclient.model.purchase.PaymentResult
import ru.rustore.sdk.billingclient.model.purchase.Purchase
import ru.rustore.sdk.billingclient.model.purchase.PurchaseState
import ru.rustore.sdk.billingclient.utils.pub.checkPurchasesAvailability

class BillingImplementation(
    val observer: BillingObserver,
) {
    private val billingClient: RuStoreBillingClient = getBillingManager()

    fun init() {
        RuStoreBillingClient.checkPurchasesAvailability(getContext())
            .addOnSuccessListener { result ->
                Log.e(getTag(), "onBillingSetupFinished $result")
                observer.onBillingSetupFinished(0)
            }
            .addOnFailureListener { throwable ->
                Log.e(getTag(), "onBillingDisconnected: $throwable ${throwable.message}")
                Log.e(getTag(), "onBillingDisconnected: $throwable ${throwable.stackTrace}")
                observer.onBillingDisconnected()
            }
    }

    fun restorePurchases() {
        billingClient.purchases.getPurchases().addOnSuccessListener{ purchases: List<Purchase> ->
            for(purchase in purchases) {
                if(PurchaseState.PAID == purchase.purchaseState) {
                    observer.onPurchaseQueried(purchase.purchaseId, purchase.productId, 0)
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
