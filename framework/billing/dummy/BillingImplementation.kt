package org.akkord.lib

import android.content.Intent

class BillingImplementation(
    val observer: BillingObserver,
) {
    fun init() {
    }

    fun restorePurchases() {
    }

    fun purchaseProdItem(productSKU: String) {
    }

    fun consumeProductItem(purchaseToken: String, productSKU: String) {
    }

    fun onNewIntent(intent: Intent) {
    }

    fun queryProductDetails(prodList: Array<String?>?) {
    }
}
