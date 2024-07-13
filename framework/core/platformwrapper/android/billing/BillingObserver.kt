package org.akkord.lib

interface BillingObserver {
    fun onBillingSetupFinished(responseCode: Int)
    fun onBillingDisconnected()
    fun onPurchaseQueried(purchaseToken: String?, productSKU: String?, type: Int)
    fun onPurchaseConsumed(purchaseToken: String?, productSKU: String?)
}