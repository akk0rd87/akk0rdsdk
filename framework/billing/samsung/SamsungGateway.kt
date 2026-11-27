package org.akkord.lib

import android.content.pm.PackageManager
import android.util.Log
import com.samsung.android.sdk.iap.lib.constants.HelperDefine
import com.samsung.android.sdk.iap.lib.helper.IapHelper
import kotlin.math.roundToLong

// Всё, что BillingImplementation (Samsung) просит у Samsung IAP SDK, - без решений: только вызовы и перекладка ответов
// в простые классы. Отдельно, чтобы логику покупок проверяли юнит-тесты (../samsung_test) с подделкой вместо Galaxy
// Store: IapHelper - синглтон, которому нужен установленный Galaxy Store.
// Коды ответов - HelperDefine (IAP_ERROR_NONE = 0, IAP_PAYMENT_IS_CANCELED = 1, остальные отрицательные); что из них
// что значит, решает BillingImplementation. Каждый вызов кончается ровно одним ответом.
interface SamsungGateway {
    // Разовые товары (PRODUCT_TYPE_ITEM), которыми пользователь владеет: непотребляемые и невыданные потребляемые
    fun ownedItems(done: (code: Int, purchases: List<SamsungPurchase>) -> Unit)
    fun products(productSKUs: List<String>, done: (code: Int, products: List<ProductInfo>) -> Unit)
    // Окно оплаты Galaxy Store; purchase - при коде IAP_ERROR_NONE
    fun purchase(productSKU: String, done: (code: Int, purchase: SamsungPurchase?) -> Unit)
    fun consume(purchaseId: String, done: (code: Int) -> Unit)
}

data class SamsungPurchase(val purchaseId: String, val productSKU: String, val consumable: Boolean)

// Режим IapHelper - из манифеста приложения: <meta-data android:name="akk0rdsdk.samsung.iap.mode" android:value="test"/>
// (test - ответы-заглушки Samsung без оплаты, test_failure - всегда ошибка, иначе - настоящие покупки)
class SamsungIapGateway : SamsungGateway {
    private val helper: IapHelper by lazy {
        IapHelper.getInstance(Utils.GetContext()).also {
            it.setOperationMode(operationMode())
            it.setShowErrorDialog(false)  // ошибки (нет сети и т.п.) игра не показывает окнами
        }
    }

    override fun ownedItems(done: (code: Int, purchases: List<SamsungPurchase>) -> Unit) = call("getOwnedList", { done(it, emptyList()) }) {
        val started = helper.getOwnedList(HelperDefine.PRODUCT_TYPE_ITEM) { error, owned ->
            done(error.errorCode, owned.orEmpty().map { SamsungPurchase(it.purchaseId, it.itemId, it.isConsumable == true) })
        }
        if (!started) done(HelperDefine.IAP_ERROR_WHILE_RUNNING, emptyList())
    }

    override fun products(productSKUs: List<String>, done: (code: Int, products: List<ProductInfo>) -> Unit) =
        call("getProductsDetails", { done(it, emptyList()) }) {
            if (productSKUs.isEmpty()) {
                done(HelperDefine.IAP_ERROR_NONE, emptyList())
                return@call
            }
            // Пустая строка - все товары приложения; список - через запятую
            helper.getProductsDetails(productSKUs.joinToString(",")) { error, products ->
                done(error.errorCode, products.orEmpty().filter { it.type == HelperDefine.PRODUCT_TYPE_ITEM }.map {
                    ProductInfo(
                        productSKU = it.itemId,
                        priceLabel = it.itemPriceString.orEmpty(),
                        priceMicros = ((it.itemPrice ?: 0.0) * 1_000_000).roundToLong(),
                        currency = it.currencyCode.orEmpty(),
                        title = it.itemName.orEmpty(),
                    )
                })
            }
        }

    override fun purchase(productSKU: String, done: (code: Int, purchase: SamsungPurchase?) -> Unit) = call("startPayment", { done(it, null) }) {
        val started = helper.startPayment(productSKU) { error, purchase ->
            done(error.errorCode, purchase?.let { SamsungPurchase(it.purchaseId, it.itemId, it.isConsumable == true) })
        }
        if (!started) done(HelperDefine.IAP_ERROR_WHILE_RUNNING, null)
    }

    override fun consume(purchaseId: String, done: (code: Int) -> Unit) = call("consumePurchasedItems", done) {
        val started = helper.consumePurchasedItems(purchaseId) { error, results ->
            // Ответ общий на список; по самому товару - его statusCode (0 - выдан)
            val item = results.orEmpty().firstOrNull { it.purchaseId == purchaseId }
            done(if (HelperDefine.IAP_ERROR_NONE == error.errorCode && item != null && item.statusCode != 0) HelperDefine.IAP_ERROR_COMMON else error.errorCode)
        }
        if (!started) done(HelperDefine.IAP_ERROR_WHILE_RUNNING)
    }

    private fun operationMode(): HelperDefine.OperationMode {
        val mode = try {
            val activity = Utils.GetContext()
            activity.packageManager.getApplicationInfo(activity.packageName, PackageManager.GET_META_DATA)
                .metaData?.getString("akk0rdsdk.samsung.iap.mode")
        } catch (e: Exception) {
            null
        }
        return when (mode) {
            "test" -> HelperDefine.OperationMode.OPERATION_MODE_TEST
            "test_failure" -> HelperDefine.OperationMode.OPERATION_MODE_TEST_FAILURE
            else -> HelperDefine.OperationMode.OPERATION_MODE_PRODUCTION
        }
    }

    private inline fun call(what: String, noinline onFailure: (Int) -> Unit, action: () -> Unit) {
        try {
            action()
        } catch (e: Exception) {
            Utils.handleException(e, what)
            Log.e(Utils.TAG, "Samsung IAP: $what: $e")
            onFailure(HelperDefine.IAP_ERROR_COMMON)
        }
    }
}
