package org.akkord.lib;

import java.util.List;

// Всё, что BillingImplementation просит у Google Play Billing Library, - без решений: только вызовы и перекладка
// ответов в простые классы ниже. Отдельно, чтобы логику покупок проверяли юнит-тесты
// (../googleplay_test/BillingImplementationTest.kt) с подделкой вместо Google Play: BillingClient - абстрактный класс
// с десятками методов, меняющихся от версии к версии, а Purchase разбирает JSON через org.json, которого на JVM нет.
// Коды ответов - BillingClient.BillingResponseCode, состояния покупки - Purchase.PurchaseState: что из них что значит,
// решает BillingImplementation. Каждый вызов кончается ровно одним ответом (исключение при вызове - код ERROR).
public interface PlayGateway {
    // События подключения и покупок, о которых Google Play сообщает сам
    interface Listener {
        void onSetupFinished(int responseCode);
        void onDisconnected();
        // Итог шторки оплаты (и покупки, завершённые вне приложения, пока оно работает); purchases может быть пустым
        void onPurchasesUpdated(int responseCode, List<PlayPurchase> purchases);
    }

    interface Done {
        void done(int responseCode);
    }

    interface PurchasesDone {
        void done(int responseCode, List<PlayPurchase> purchases);
    }

    interface ProductsDone {
        void done(int responseCode, List<ProductInfo> products);
    }

    // Подключение с автоматическим переподключением; ответ - Listener.onSetupFinished
    void connect(Listener listener);
    // Разовые товары (INAPP), которыми пользователь владеет сейчас
    void queryPurchases(PurchasesDone done);
    // Узнаёт товар у Google Play и открывает шторку оплаты. done - OK, если шторка открылась (итог оплаты придёт в
    // Listener.onPurchasesUpdated); ITEM_UNAVAILABLE - Google Play не знает такого товара; иначе код ошибки
    void launchPurchase(String productSKU, Done done);
    // Разовые товары (INAPP) с ценами; товаров, которых Google Play не знает, в ответе нет
    void queryProducts(List<String> productSKUs, ProductsDone done);
    void acknowledge(String purchaseToken, Done done);
    void consume(String purchaseToken, Done done);

    // Покупка, какой её знает Google Play
    final class PlayPurchase {
        public final String purchaseToken;
        public final List<String> products;
        public final int state;              // Purchase.PurchaseState
        public final boolean acknowledged;
        public final String orderId;         // для лога; у отложенной покупки его нет

        public PlayPurchase(String purchaseToken, List<String> products, int state, boolean acknowledged, String orderId) {
            this.purchaseToken = purchaseToken;
            this.products = products;
            this.state = state;
            this.acknowledged = acknowledged;
            this.orderId = orderId;
        }
    }
}
