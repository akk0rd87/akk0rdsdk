package org.akkord.lib

import android.content.Intent
import android.util.Log
import org.akkord.lib.BillingImplementation

private external fun billingSetupFinished()
private external fun billingDisconnected()
private external fun purchaseQueried(purchaseToken: String?, productSKU: String?, type: Int)
private external fun purchaseConsumed(purchaseToken: String?, productSKU: String?)
private external fun purchasesQueryFinished(success: Boolean)
private external fun productDetailsReceived(skus: Array<String>, priceLabels: Array<String>, priceMicros: LongArray,
                                            currencies: Array<String>, titles: Array<String>)

class BillingManager  {
    companion object BillingObserverImpl : BillingObserver {
        const val TAG: String = "SDL"
        private val mBillingImpl by lazy { BillingImplementation(this) }

        @JvmStatic
        fun initialize() {
            try {
                mBillingImpl.init()
            }
            catch(e : Exception) {
                Utils.handleException(e, "initialize")
            }
        }

        @JvmStatic
        fun restorePurchases() {
            try {
                mBillingImpl.restorePurchases()
            }
            catch(e : Exception) {
                Utils.handleException(e, "restorePurchases")
            }
        }

        @JvmStatic
        fun purchaseProdItem(productSKU: String) {
            try {
                mBillingImpl.purchaseProdItem(productSKU)
            }
            catch(e : Exception) {
                Utils.handleException(e, "purchaseProdItem")
            }
        }

        @JvmStatic
        fun consumeProductItem(purchaseToken: String, productSKU: String) {
            try {
                mBillingImpl.consumeProductItem(purchaseToken, productSKU)
            }
            catch(e : Exception) {
                Utils.handleException(e, "consumeProductItem")
            }
        }

        @JvmStatic
        fun queryProductDetails(prodList: Array<String?>?) {
            try {
                mBillingImpl.queryProductDetails(prodList)
            }
            catch(e : Exception) {
                Utils.handleException(e, "queryProductDetails")
            }
        }

        @JvmStatic
        fun onNewIntent(intent: Intent) {
            try {
                mBillingImpl.onNewIntent(intent)
            }
            catch(e : Exception) {
                Utils.handleException(e, "onNewIntent")
            }
        }

        override fun onBillingSetupFinished() {
            try {
                billingSetupFinished()
                restorePurchases()
            } catch(e: Exception) {
                Utils.handleException(e, "onBillingSetupFinished")
            } catch(e: UnsatisfiedLinkError) {
                Utils.handleException(e, "onBillingSetupFinished")
            }
        }

        override fun onBillingDisconnected() {
            try {
                billingDisconnected()
            } catch(e: Exception) {
                Utils.handleException(e, "onBillingDisconnected")
            } catch(e: UnsatisfiedLinkError) {
                Utils.handleException(e, "onBillingDisconnected")
            }
        }

        override fun onPurchaseQueried(purchaseToken: String?, productSKU: String?, type: Int) {
            try {
                purchaseQueried(purchaseToken, productSKU, type)
            } catch(e: Exception) {
                Utils.handleException(e, "onPurchaseQueried")
            } catch(e: UnsatisfiedLinkError) {
                Utils.handleException(e, "onPurchaseQueried")
            }
        }

        override fun onPurchasesQueryFinished(success: Boolean) {
            try {
                purchasesQueryFinished(success)
            } catch(e: Exception) {
                Utils.handleException(e, "onPurchasesQueryFinished")
            } catch(e: UnsatisfiedLinkError) {
                Utils.handleException(e, "onPurchasesQueryFinished")
            }
        }

        override fun onProductDetails(products: List<ProductInfo>) {
            try {
                productDetailsReceived(
                    products.map { it.productSKU }.toTypedArray(),
                    products.map { it.priceLabel }.toTypedArray(),
                    products.map { it.priceMicros }.toLongArray(),
                    products.map { it.currency }.toTypedArray(),
                    products.map { it.title }.toTypedArray(),
                )
            } catch(e: Exception) {
                Utils.handleException(e, "onProductDetails")
            } catch(e: UnsatisfiedLinkError) {
                Utils.handleException(e, "onProductDetails")
            }
        }

        override fun onPurchaseConsumed(purchaseToken: String?, productSKU: String?) {
            try {
                purchaseConsumed(purchaseToken, productSKU)
            } catch(e: Exception) {
                Utils.handleException(e, "onPurchaseConsumed")
            } catch(e: UnsatisfiedLinkError) {
                Utils.handleException(e, "onPurchaseConsumed")
            }
        }
    }
}