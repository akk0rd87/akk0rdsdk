package org.akkord.lib;

import android.app.Activity;
import android.util.Log;

import androidx.annotation.NonNull;

import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClient.BillingResponseCode;
import com.android.billingclient.api.BillingClient.ProductType;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.PendingPurchasesParams;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryPurchasesParams;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

// PlayGateway на Google Play Billing Library: см. PlayGateway.java
public class PlayBillingGateway implements PlayGateway {
    private BillingClient mBillingClient = null;

    @Override
    public void connect(final Listener listener) {
        try {
            mBillingClient = BillingClient.newBuilder(getContext())
                    .setListener((billingResult, purchases) ->
                            listener.onPurchasesUpdated(billingResult.getResponseCode(), toPlayPurchases(purchases)))
                    .enablePendingPurchases(PendingPurchasesParams.newBuilder().enableOneTimeProducts().enablePrepaidPlans().build())
                    .enableAutoServiceReconnection()
                    .build();

            mBillingClient.startConnection(new BillingClientStateListener() {
                @Override
                public void onBillingSetupFinished(@NonNull BillingResult billingResult) {
                    listener.onSetupFinished(billingResult.getResponseCode());
                }

                @Override
                public void onBillingServiceDisconnected() {
                    listener.onDisconnected();
                }
            });
        }
        catch(Exception e) {
            Log.e(getTag(), "Billing connect: " + e);
            listener.onSetupFinished(BillingResponseCode.ERROR);
        }
    }

    @Override
    public void queryPurchases(final PurchasesDone done) {
        try {
            mBillingClient.queryPurchasesAsync(
                QueryPurchasesParams.newBuilder().setProductType(ProductType.INAPP).build(),
                (billingResult, purchases) -> done.done(billingResult.getResponseCode(), toPlayPurchases(purchases))
            );
        }
        catch(Exception e) {
            Log.e(getTag(), "queryPurchases: " + e);
            done.done(BillingResponseCode.ERROR, Collections.emptyList());
        }
    }

    @Override
    public void launchPurchase(final String productSKU, final Done done) {
        try {
            final QueryProductDetailsParams params = QueryProductDetailsParams.newBuilder()
                .setProductList(List.of(QueryProductDetailsParams.Product.newBuilder()
                    .setProductType(ProductType.INAPP)
                    .setProductId(productSKU)
                    .build()))
                .build();

            mBillingClient.queryProductDetailsAsync(params, (billingResult, queryProductDetailsResult) -> {
                try {
                    if (BillingResponseCode.OK != billingResult.getResponseCode()) {
                        Log.e(getTag(), "queryProductDetails " + productSKU + ": " + billingResult.getDebugMessage());
                        done.done(billingResult.getResponseCode());
                        return;
                    }
                    final List<ProductDetails> details = queryProductDetailsResult.getProductDetailsList();
                    if (details.isEmpty()) {
                        done.done(BillingResponseCode.ITEM_UNAVAILABLE);
                        return;
                    }
                    final BillingFlowParams flowParams = BillingFlowParams.newBuilder()
                        .setProductDetailsParamsList(List.of(
                            BillingFlowParams.ProductDetailsParams.newBuilder().setProductDetails(details.get(0)).build()))
                        .build();
                    // Шторка - это Activity: открываем с UI-потока
                    getContext().runOnUiThread(() -> {
                        try {
                            done.done(mBillingClient.launchBillingFlow(getContext(), flowParams).getResponseCode());
                        }
                        catch(Exception e) {
                            Log.e(getTag(), "launchBillingFlow: " + e);
                            done.done(BillingResponseCode.ERROR);
                        }
                    });
                }
                catch(Exception e) {
                    Log.e(getTag(), "queryProductDetails: " + e);
                    done.done(BillingResponseCode.ERROR);
                }
            });
        }
        catch(Exception e) {
            Log.e(getTag(), "launchPurchase: " + e);
            done.done(BillingResponseCode.ERROR);
        }
    }

    @Override
    public void queryProducts(final List<String> productSKUs, final ProductsDone done) {
        try {
            final List<QueryProductDetailsParams.Product> products = new ArrayList<>();
            for (String sku : productSKUs) {
                products.add(QueryProductDetailsParams.Product.newBuilder().setProductType(ProductType.INAPP).setProductId(sku).build());
            }
            if (products.isEmpty()) {
                done.done(BillingResponseCode.OK, Collections.emptyList());
                return;
            }
            mBillingClient.queryProductDetailsAsync(
                QueryProductDetailsParams.newBuilder().setProductList(products).build(),
                (billingResult, result) -> {
                    final List<ProductInfo> out = new ArrayList<>();
                    for (ProductDetails details : result.getProductDetailsList()) {
                        final ProductDetails.OneTimePurchaseOfferDetails offer = details.getOneTimePurchaseOfferDetails();
                        out.add(new ProductInfo(
                            details.getProductId(),
                            offer != null ? offer.getFormattedPrice() : "",
                            offer != null ? offer.getPriceAmountMicros() : 0L,
                            offer != null ? offer.getPriceCurrencyCode() : "",
                            details.getName()));
                    }
                    done.done(billingResult.getResponseCode(), out);
                });
        }
        catch(Exception e) {
            Log.e(getTag(), "queryProducts: " + e);
            done.done(BillingResponseCode.ERROR, Collections.emptyList());
        }
    }

    @Override
    public void acknowledge(final String purchaseToken, final Done done) {
        try {
            mBillingClient.acknowledgePurchase(
                AcknowledgePurchaseParams.newBuilder().setPurchaseToken(purchaseToken).build(),
                billingResult -> done.done(billingResult.getResponseCode()));
        }
        catch(Exception e) {
            Log.e(getTag(), "acknowledge: " + e);
            done.done(BillingResponseCode.ERROR);
        }
    }

    @Override
    public void consume(final String purchaseToken, final Done done) {
        try {
            mBillingClient.consumeAsync(
                ConsumeParams.newBuilder().setPurchaseToken(purchaseToken).build(),
                (billingResult, token) -> done.done(billingResult.getResponseCode()));
        }
        catch(Exception e) {
            Log.e(getTag(), "consume: " + e);
            done.done(BillingResponseCode.ERROR);
        }
    }

    private static List<PlayPurchase> toPlayPurchases(List<Purchase> purchases) {
        final List<PlayPurchase> out = new ArrayList<>();
        if (purchases != null) {
            for (Purchase p : purchases) {
                if (p != null) {
                    out.add(new PlayPurchase(p.getPurchaseToken(), p.getProducts(), p.getPurchaseState(), p.isAcknowledged(), p.getOrderId()));
                }
            }
        }
        return out;
    }

    private static String getTag() {
        return org.akkord.lib.Utils.TAG;
    }

    private static Activity getContext() {
        return org.akkord.lib.Utils.GetContext();
    }
}
