package org.akkord.lib

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.util.Log
import com.huawei.hms.iap.Iap
import com.huawei.hms.iap.IapApiException
import com.huawei.hms.iap.IapClient
import com.huawei.hms.iap.entity.ConsumeOwnedPurchaseReq
import com.huawei.hms.iap.entity.InAppPurchaseData
import com.huawei.hms.iap.entity.OrderStatusCode
import com.huawei.hms.iap.entity.OwnedPurchasesReq
import com.huawei.hms.iap.entity.ProductInfoReq
import com.huawei.hms.iap.entity.PurchaseIntentReq
import com.huawei.hms.support.api.client.Status

// Всё, что BillingImplementation (Huawei) просит у HMS IAP Kit, - без решений: только вызовы и перекладка ответов в
// простые классы. Отдельно, чтобы логику покупок проверяли юнит-тесты (../huawei_test) с подделкой вместо HMS:
// клиент IAP - интерфейс с задачами HMS, а покупки приходят JSON-строками (org.json на JVM нет).
// Коды ответов - OrderStatusCode (0 - успех); что из них что значит, решает BillingImplementation. Каждый вызов
// кончается ровно одним ответом (исключение - код ORDER_STATE_FAILED или код из IapApiException).
interface HmsGateway {
    // isEnvReady: вошёл ли пользователь в аккаунт Huawei и есть ли IAP в его стране. Окон не показывает
    fun checkEnvironment(done: (code: Int) -> Unit)
    // Окно входа в аккаунт Huawei (после checkEnvironment с ORDER_HWID_NOT_LOGIN): 0 - вошёл
    fun signIn(done: (code: Int) -> Unit)
    // Купленное, со всех страниц (continuationToken); priceType - IapClient.PriceType
    fun ownedPurchases(priceType: Int, done: (code: Int, purchases: List<HmsPurchase>) -> Unit)
    // Товары этого вида с ценами; товара другого вида или неизвестного в ответе нет
    fun products(priceType: Int, productSKUs: List<String>, done: (code: Int, products: List<ProductInfo>) -> Unit)
    // Шторка оплаты; purchase - при коде 0
    fun purchase(productSKU: String, priceType: Int, done: (code: Int, purchase: HmsPurchase?) -> Unit)
    fun consume(purchaseToken: String, done: (code: Int) -> Unit)
}

// Покупка, какой её знает HMS
data class HmsPurchase(
    val purchaseToken: String,
    val productSKU: String,
    val state: Int,       // InAppPurchaseData.PurchaseState
    val priceType: Int,   // IapClient.PriceType
)

class HmsIapGateway : HmsGateway {
    // Статус последнего отказа isEnvReady с окном решения (вход в аккаунт): его и показывает signIn
    private var signInStatus: Status? = null

    override fun checkEnvironment(done: (code: Int) -> Unit) = call("isEnvReady", done) {
        client().isEnvReady()
            .addOnSuccessListener { signInStatus = null; done(OrderStatusCode.ORDER_STATE_SUCCESS) }
            .addOnFailureListener { e ->
                signInStatus = (e as? IapApiException)?.status?.takeIf { it.hasResolution() }
                done(codeOf(e))
            }
    }

    override fun signIn(done: (code: Int) -> Unit) {
        val status = signInStatus
        if (status == null) {
            done(OrderStatusCode.ORDER_HWID_NOT_LOGIN)
            return
        }
        HuaweiIapActivity.resolve(status) { resultCode, _ ->
            done(if (Activity.RESULT_OK == resultCode) OrderStatusCode.ORDER_STATE_SUCCESS else OrderStatusCode.ORDER_STATE_CANCEL)
        }
    }

    override fun ownedPurchases(priceType: Int, done: (code: Int, purchases: List<HmsPurchase>) -> Unit) {
        val all = mutableListOf<HmsPurchase>()
        fun page(token: String?) {
            call("obtainOwnedPurchases", { code -> done(code, emptyList()) }) {
                val req = OwnedPurchasesReq()
                req.priceType = priceType
                if (token != null) req.continuationToken = token
                client().obtainOwnedPurchases(req)
                    .addOnSuccessListener { result ->
                        for (json in result.inAppPurchaseDataList.orEmpty()) {
                            try {
                                val data = InAppPurchaseData(json)
                                all += HmsPurchase(data.purchaseToken, data.productId, data.purchaseState, priceType)
                            } catch (e: Exception) {
                                Log.e(Utils.TAG, "HMS IAP: purchase data: $e")
                            }
                        }
                        val next = result.continuationToken
                        if (next.isNullOrEmpty()) done(OrderStatusCode.ORDER_STATE_SUCCESS, all) else page(next)
                    }
                    .addOnFailureListener { e -> done(codeOf(e), emptyList()) }
            }
        }
        page(null)
    }

    override fun products(priceType: Int, productSKUs: List<String>, done: (code: Int, products: List<ProductInfo>) -> Unit) =
        call("obtainProductInfo", { code -> done(code, emptyList()) }) {
            if (productSKUs.isEmpty()) {
                done(OrderStatusCode.ORDER_STATE_SUCCESS, emptyList())
                return@call
            }
            val req = ProductInfoReq()
            req.priceType = priceType
            req.productIds = productSKUs
            client().obtainProductInfo(req)
                .addOnSuccessListener { result ->
                    done(OrderStatusCode.ORDER_STATE_SUCCESS, result.productInfoList.orEmpty().map {
                        ProductInfo(it.productId, it.price.orEmpty(), it.microsPrice, it.currency.orEmpty(), it.productName.orEmpty())
                    })
                }
                .addOnFailureListener { e -> done(codeOf(e), emptyList()) }
        }

    override fun purchase(productSKU: String, priceType: Int, done: (code: Int, purchase: HmsPurchase?) -> Unit) =
        call("createPurchaseIntent", { code -> done(code, null) }) {
            val req = PurchaseIntentReq()
            req.productId = productSKU
            req.priceType = priceType
            client().createPurchaseIntent(req)
                .addOnSuccessListener { result ->
                    val status = result.status
                    if (status == null || !status.hasResolution()) {
                        done(OrderStatusCode.ORDER_STATE_FAILED, null)
                        return@addOnSuccessListener
                    }
                    HuaweiIapActivity.resolve(status) { _, data ->
                        try {
                            val info = client().parsePurchaseResultInfoFromIntent(data)
                            val purchase = if (OrderStatusCode.ORDER_STATE_SUCCESS == info.returnCode) {
                                val d = InAppPurchaseData(info.inAppPurchaseData)
                                HmsPurchase(d.purchaseToken, d.productId, d.purchaseState, priceType)
                            } else null
                            done(info.returnCode, purchase)
                        } catch (e: Exception) {
                            Log.e(Utils.TAG, "HMS IAP: purchase result: $e")
                            done(OrderStatusCode.ORDER_STATE_FAILED, null)
                        }
                    }
                }
                .addOnFailureListener { e -> done(codeOf(e), null) }
        }

    override fun consume(purchaseToken: String, done: (code: Int) -> Unit) = call("consumeOwnedPurchase", done) {
        val req = ConsumeOwnedPurchaseReq()
        req.purchaseToken = purchaseToken
        client().consumeOwnedPurchase(req)
            .addOnSuccessListener { done(OrderStatusCode.ORDER_STATE_SUCCESS) }
            .addOnFailureListener { e -> done(codeOf(e)) }
    }

    private fun codeOf(e: Exception): Int = (e as? IapApiException)?.statusCode ?: OrderStatusCode.ORDER_STATE_FAILED

    private inline fun call(what: String, noinline onFailure: (Int) -> Unit, action: () -> Unit) {
        try {
            action()
        } catch (e: Exception) {
            Utils.handleException(e, what)
            onFailure(OrderStatusCode.ORDER_STATE_FAILED)
        }
    }

    private fun client(): IapClient = Iap.getIapClient(Utils.GetContext())
}

// Прозрачная активити-посредник: шторка оплаты и вход в аккаунт Huawei отдают результат в onActivityResult, а
// SDL-активити (AkkordActivity) его не принимает и Activity Result API у неё нет. Приложение объявляет её в манифесте
// флейвора huaweiStore:
//   <activity android:name="org.akkord.lib.HuaweiIapActivity" android:exported="false"
//             android:theme="@android:style/Theme.Translucent.NoTitleBar" android:configChanges="orientation|screenSize|keyboardHidden"/>
class HuaweiIapActivity : Activity() {
    companion object {
        private const val REQUEST = 7301
        private var pending: Pair<Status, (resultCode: Int, data: Intent?) -> Unit>? = null

        fun resolve(status: Status, done: (resultCode: Int, data: Intent?) -> Unit) {
            val activity = Utils.GetContext()
            pending = status to done
            activity.runOnUiThread { activity.startActivity(Intent(activity, HuaweiIapActivity::class.java)) }
        }
    }

    private var answered = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (savedInstanceState != null) return  // пересоздана системой: ответ придёт в onActivityResult
        val status = pending?.first
        if (status == null) {
            finish()
            return
        }
        try {
            status.startResolutionForResult(this, REQUEST)
        } catch (e: Exception) {
            Log.e(Utils.TAG, "HMS IAP: startResolutionForResult: $e")
            answer(RESULT_CANCELED, null)
        }
    }

    @Deprecated("Activity Result API needs ComponentActivity; this activity stays a plain one on purpose")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (REQUEST == requestCode) answer(resultCode, data)
    }

    override fun onDestroy() {
        super.onDestroy()
        if (isFinishing && !answered) answer(RESULT_CANCELED, null)
    }

    private fun answer(resultCode: Int, data: Intent?) {
        if (answered) return
        answered = true
        val done = pending?.second
        pending = null
        done?.invoke(resultCode, data)
        finish()
    }
}
