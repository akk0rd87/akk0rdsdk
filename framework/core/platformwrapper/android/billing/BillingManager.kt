package org.akkord.lib

import android.content.Intent
import android.util.Log
import org.akkord.lib.BillingImplementation

private external fun billingSetupFinished(responseCode: Int)
private external fun billingDisconnected()
private external fun purchaseQueried(purchaseToken: String?, productSKU: String?, type: Int)
private external fun purchaseConsumed(purchaseToken: String?, productSKU: String?)

class BillingManager  {
    val PURCHASE_RESTORED: Int = 0
    val PURCHASE_BOUGHT: Int = 1

    companion object BillingObserverImpl : BillingObserver {
        const val TAG: String = "SDL"
        private val mBillingImpl by lazy { BillingImplementation(this) }

        @JvmStatic
        fun initialize() {
            try {
                mBillingImpl.init()
            }
            catch(e : Exception) {
                e.message?.let { Log.e(TAG, it) }
            }
        }

        @JvmStatic
        fun restorePurchases() {
            try {
                mBillingImpl.restorePurchases()
            }
            catch(e : Exception) {
                e.message?.let { Log.e(TAG, it) }
            }
        }

        @JvmStatic
        fun purchaseProdItem(productSKU: String) {
            try {
                mBillingImpl.purchaseProdItem(productSKU)
            }
            catch(e : Exception) {
                e.message?.let { Log.e(TAG, it) }
            }
        }

        @JvmStatic
        fun consumeProductItem(purchaseToken: String, productSKU: String) {
            try {
                mBillingImpl.consumeProductItem(purchaseToken, productSKU)
            }
            catch(e : Exception) {
                e.message?.let { Log.e(TAG, it) }
            }
        }

        @JvmStatic
        fun queryProductDetails(prodList: Array<String?>?) {
            try {
                mBillingImpl.queryProductDetails(prodList)
            }
            catch(e : Exception) {
                e.message?.let { Log.e(TAG, it) }
            }
        }

        @JvmStatic
        fun onNewIntent(intent: Intent) {
            try {
                mBillingImpl.onNewIntent(intent)
            }
            catch(e : Exception) {
                e.message?.let { Log.e(TAG, it) }
            }
        }

        override fun onBillingSetupFinished(responseCode: Int) {
            billingSetupFinished(responseCode)
        }

        override fun onBillingDisconnected() {
            billingDisconnected()
        }

        override fun onPurchaseQueried(purchaseToken: String?, productSKU: String?, type: Int) {
            purchaseQueried(purchaseToken, productSKU, type)
        }

        override fun onPurchaseConsumed(purchaseToken: String?, productSKU: String?) {
            purchaseConsumed(purchaseToken, productSKU)
        }
    }
}