package org.akkord.lib;

import android.content.Intent;
import android.util.Log;

import com.android.billingclient.api.BillingClient.BillingResponseCode;
import com.android.billingclient.api.Purchase;

import java.util.ArrayList;
import java.util.List;

// Биллинг Google Play. Товары: непотребляемые - restorePurchases сообщает о них при каждом вызове; потребляемые - до
// consumeProductItem. Новую покупку SDK подтверждает (acknowledge) сам: неподтверждённую Google через 3 дня вернёт.
// Подписок пока нет. Здесь только решения, вызовы Google Play - в PlayGateway (PlayBillingGateway); тесты -
// ../googleplay_test/BillingImplementationTest.kt.
public class BillingImplementation implements PlayGateway.Listener {
    public static final int PURCHASE_RESTORED = 0;
    public static final int PURCHASE_BOUGHT   = 1;

    private final BillingObserver mBillingObserver;
    private final PlayGateway mPlay;

    public BillingImplementation(BillingObserver observer) {
        this(observer, new PlayBillingGateway());
    }

    public BillingImplementation(BillingObserver observer, PlayGateway play) {
        mBillingObserver = observer;
        mPlay = play;
        mPlay.connect(this);
    }

    public void onNewIntent(Intent intent) {

    }

    // Подключение начинается в конструкторе, о готовности сообщит onSetupFinished
    public void init() {

    }

    // Товары с ценами для показа в игре -> onProductDetails (пустой список - Google Play не ответил)
    public void queryProductDetails(final String[] ProdList) {
        final List<String> skus = new ArrayList<>();
        if (ProdList != null) {
            for (String sku : ProdList) {
                if (sku != null && !sku.isEmpty()) skus.add(sku);
            }
        }
        mPlay.queryProducts(skus, (responseCode, products) -> {
            if (BillingResponseCode.OK != responseCode) {
                Log.e(getTag(), "queryProductDetails: " + DecodeBillingResponse(responseCode));
                mBillingObserver.onProductDetails(new ArrayList<>());
                return;
            }
            mBillingObserver.onProductDetails(products);
        });
    }

    // Подписки в Google Play пока не поддерживаются
    public void querySubscriptionProducts(final String[] ProdList) {

    }

    public void purchaseSubscription(final String ProductSKU) {

    }

    public void restorePurchases() {
        mPlay.queryPurchases((responseCode, purchases) -> {
            checkPurchases(responseCode, purchases);
            // Только здесь, не в onPurchasesUpdated: туда приходит не полный список покупок.
            // Неподтверждённые покупки дойдут до наблюдателя позже, после acknowledge
            mBillingObserver.onPurchasesQueryFinished(BillingResponseCode.OK == responseCode);
        });
    }

    public void purchaseProdItem(final String ProductSKU) {
        mPlay.launchPurchase(ProductSKU, responseCode -> {
            // Итог оплаты придёт в onPurchasesUpdated; здесь - только почему шторка не открылась
            if (BillingResponseCode.OK != responseCode) {
                Log.e(getTag(), "Purchase " + ProductSKU + " did not start: " + DecodeBillingResponse(responseCode));
            }
        });
    }

    public void consumeProductItem(final String PurchaseToken, final String SKU) {
        Log.v(getTag(), "Consume: " + SKU + " " + PurchaseToken);
        mPlay.consume(PurchaseToken, responseCode -> {
            Log.v(getTag(), "Consume " + SKU + ": " + DecodeBillingResponse(responseCode));
            if (BillingResponseCode.OK == responseCode) {
                mBillingObserver.onPurchaseConsumed(PurchaseToken, SKU);
            }
        });
    }

    // ---- PlayGateway.Listener ----

    @Override
    public void onSetupFinished(int responseCode) {
        Log.v(getTag(), "Setup finished. Response: " + DecodeBillingResponse(responseCode));
        if (BillingResponseCode.OK == responseCode) {
            mBillingObserver.onBillingSetupFinished();
        } else {
            mBillingObserver.onBillingDisconnected();
        }
    }

    @Override
    public void onDisconnected() {
        Log.v(getTag(), "onBillingServiceDisconnected");
        mBillingObserver.onBillingDisconnected();
    }

    @Override
    public void onPurchasesUpdated(int responseCode, List<PlayGateway.PlayPurchase> purchases) {
        if (BillingResponseCode.ITEM_ALREADY_OWNED == responseCode) {
            // Куплено раньше (на другом устройстве, или подтверждение не дошло): спросить у магазина, что куплено,
            // - покупка придёт игре как восстановленная (и неподтверждённая будет подтверждена)
            restorePurchases();
            return;
        }
        checkPurchases(responseCode, purchases);
    }

    // ---- решения ----

    private void checkPurchases(int responseCode, List<PlayGateway.PlayPurchase> purchases) {
        if (BillingResponseCode.OK != responseCode) {
            Log.v(getTag(), "checkPurchases error, response = " + DecodeBillingResponse(responseCode));
            return;
        }
        for (PlayGateway.PlayPurchase purchase : purchases) {
            switch (purchase.state) {
                case Purchase.PurchaseState.PURCHASED:
                    if (purchase.acknowledged) {
                        Log.v(getTag(), "Purchase was Bought: " + purchase.purchaseToken + " Order:" + purchase.orderId);
                        reportProducts(purchase, PURCHASE_RESTORED);
                    } else {
                        acknowledge(purchase);
                    }
                    break;

                case Purchase.PurchaseState.PENDING:
                    // Оплата ещё не прошла (наличные, перевод): придёт в onPurchasesUpdated или при следующем restorePurchases
                    Log.v(getTag(), "Purchase pending: " + purchase.purchaseToken);
                    break;

                default:
                    Log.v(getTag(), "Purchase unspecified_state:" + purchase.purchaseToken + " Order:" + purchase.orderId);
                    break;
            }
        }
    }

    private void acknowledge(final PlayGateway.PlayPurchase purchase) {
        mPlay.acknowledge(purchase.purchaseToken, responseCode -> {
            if (BillingResponseCode.OK != responseCode) {
                // Неподтверждённую покупку Google через 3 дня вернёт: повтор - при следующем
                // restorePurchases (старт, «уже куплено»); товар игре отдаётся сразу - деньги списаны.
                Log.e(getTag(), "acknowledgePurchase: " + DecodeBillingResponse(responseCode));
            }
            reportProducts(purchase, PURCHASE_BOUGHT);
        });
    }

    private void reportProducts(final PlayGateway.PlayPurchase purchase, int BoughtOrRestored) {
        for (String product : purchase.products) {
            if (product != null && !product.isEmpty()) {
                Log.v(getTag(), "sku:" + product);
                mBillingObserver.onPurchaseQueried(purchase.purchaseToken, product, BoughtOrRestored);
            }
        }
    }

    static String DecodeBillingResponse(int billingResponseCode) {
        switch (billingResponseCode) {
            case BillingResponseCode.BILLING_UNAVAILABLE: return "BILLING_UNAVAILABLE";
            case BillingResponseCode.DEVELOPER_ERROR: return "DEVELOPER_ERROR";
            case BillingResponseCode.ERROR: return "ERROR";
            case BillingResponseCode.FEATURE_NOT_SUPPORTED: return "FEATURE_NOT_SUPPORTED";
            case BillingResponseCode.ITEM_ALREADY_OWNED: return "ITEM_ALREADY_OWNED";
            case BillingResponseCode.ITEM_NOT_OWNED: return "ITEM_NOT_OWNED";
            case BillingResponseCode.ITEM_UNAVAILABLE: return "ITEM_UNAVAILABLE";
            case BillingResponseCode.OK: return "OK";
            case BillingResponseCode.SERVICE_DISCONNECTED: return "SERVICE_DISCONNECTED";
            case BillingResponseCode.SERVICE_UNAVAILABLE: return "SERVICE_UNAVAILABLE";
            case BillingResponseCode.USER_CANCELED: return "USER_CANCELED";
            case BillingResponseCode.NETWORK_ERROR: return "NETWORK_ERROR";
            default: return "UNKNOWN(" + billingResponseCode + ")";
        }
    }

    private static String getTag() {
        return org.akkord.lib.Utils.TAG;
    }
}
