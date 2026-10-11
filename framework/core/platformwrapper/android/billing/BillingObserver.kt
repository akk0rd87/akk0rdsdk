package org.akkord.lib

interface BillingObserver {
    fun onBillingSetupFinished()
    fun onBillingDisconnected()
    fun onPurchaseQueried(purchaseToken: String?, productSKU: String?, type: Int)
    fun onPurchaseConsumed(purchaseToken: String?, productSKU: String?)

    // Запрос списка покупок (restorePurchases) завершён: все onPurchaseQueried / onSubscriptionStatus по нему уже вызваны.
    // success == false — ответ от стора не получен (нет сети, пользователь не авторизован и т.п.),
    // отсутствие покупок в этом случае ничего не значит
    fun onPurchasesQueryFinished(success: Boolean) {}

    // Статус подписки пользователя. expirationMs — время окончания оплаченного периода (epoch, мс),
    // 0 — стор не сообщает его клиенту
    fun onSubscriptionStatus(productSKU: String, active: Boolean, expirationMs: Long) {}

    // Ответ на querySubscriptionProducts: тарифы, доступные для покупки (пустой список — стор ничего не вернул)
    fun onSubscriptionProducts(products: List<SubscriptionProduct>) {}
}

// Тариф подписки в независимом от стора виде
class SubscriptionProduct(
    val productSKU: String,
    val priceLabel: String,  // цена основного периода с символом валюты, как её отдаёт стор
    val priceMinor: Long,    // цена основного периода в минимальных единицах валюты (копейках), 0 — неизвестна
    val period: String,      // длительность основного периода в ISO 8601 (P1M, P1Y), пусто — неизвестна
    val trialPeriod: String, // длительность пробного периода в ISO 8601, пусто — пробного периода нет
)
