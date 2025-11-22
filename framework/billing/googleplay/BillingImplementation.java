package org.akkord.lib;

import android.app.Activity;
import android.content.Intent;

import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.BillingClient.BillingResponseCode;
import com.android.billingclient.api.BillingClient.ProductType;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.PendingPurchasesParams;
import com.android.billingclient.api.PurchasesResponseListener;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.ConsumeResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryPurchasesParams;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.PurchaseHistoryRecord;
import com.android.billingclient.api.PurchaseHistoryResponseListener;
import java.util.Objects;
import android.util.Log;
import androidx.annotation.NonNull;
import java.util.List;

public class BillingImplementation implements PurchaseHistoryResponseListener, PurchasesUpdatedListener, BillingClientStateListener, PurchasesResponseListener {
    private BillingObserver mBillingObserver = null;
    private BillingClient mBillingClient = null;

    private final int PURCHASE_RESTORED = 0;
    private final int PURCHASE_BOUGHT   = 1;

    public BillingImplementation(BillingObserver observer){
        mBillingObserver = observer;

        mBillingClient = BillingClient.newBuilder(getContext())
                .setListener(this) // PurchasesUpdatedListener
                .enablePendingPurchases(PendingPurchasesParams.newBuilder().enableOneTimeProducts().enablePrepaidPlans().build())
                .enableAutoServiceReconnection()
                .build();

        mBillingClient.startConnection(this);
    }

    public void onNewIntent(Intent intent) {

    }

    public void init() {

    }

    private static String DecodeBillingResponse(int billingResponseCode)
    {
        String StringCode = "UNKNOWN";
        switch(billingResponseCode)
        {
            case BillingResponseCode.BILLING_UNAVAILABLE: StringCode = "BILLING_UNAVAILABLE"; break;
            case BillingResponseCode.DEVELOPER_ERROR: StringCode = "DEVELOPER_ERROR"; break;
            case BillingResponseCode.ERROR: StringCode = "ERROR"; break;
            case BillingResponseCode.FEATURE_NOT_SUPPORTED: StringCode = "FEATURE_NOT_SUPPORTED"; break;
            case BillingResponseCode.ITEM_ALREADY_OWNED: StringCode = "ITEM_ALREADY_OWNED"; break;
            case BillingResponseCode.ITEM_NOT_OWNED: StringCode = "ITEM_NOT_OWNED"; break;
            case BillingResponseCode.ITEM_UNAVAILABLE: StringCode = "ITEM_UNAVAILABLE"; break;
            case BillingResponseCode.OK: StringCode = "OK"; break;
            case BillingResponseCode.SERVICE_DISCONNECTED: StringCode = "SERVICE_DISCONNECTED"; break;
            case BillingResponseCode.SERVICE_UNAVAILABLE: StringCode = "SERVICE_UNAVAILABLE"; break;
            case BillingResponseCode.USER_CANCELED: StringCode = "USER_CANCELED"; break;
            case BillingResponseCode.NETWORK_ERROR: StringCode = "NETWORK_ERROR"; break;
        }

        return StringCode;
    }

    public void queryProductDetails(final String[] ProdList) {

    }

    public void restorePurchases() {
        try {
            mBillingClient.queryPurchasesAsync(
                QueryPurchasesParams.newBuilder().setProductType(ProductType.INAPP).build(),
                this
            );
        }
        catch(Exception e) {
            Log.e(getTag(), e.toString());
        }
    }

    public void purchaseProdItem(final String ProductSKU) {
        try {
            final List<QueryProductDetailsParams.Product> productList = List.of(QueryProductDetailsParams.Product.newBuilder()
                                                        .setProductType(ProductType.INAPP)
                                                        .setProductId(ProductSKU)
                                                        .build());

            final QueryProductDetailsParams params = QueryProductDetailsParams.newBuilder()
                .setProductList(productList)
                .build();

            mBillingClient.queryProductDetailsAsync(params,
                (billingResult, queryProductDetailsResult) -> {
                    final List<ProductDetails> productDetailsList = queryProductDetailsResult.getProductDetailsList();
                    if(BillingResponseCode.OK == billingResult.getResponseCode() && !productDetailsList.isEmpty()) {
                        final List<BillingFlowParams.ProductDetailsParams> productDetailsParamsList =
                            List.of(
                                BillingFlowParams.ProductDetailsParams.newBuilder()
                                    .setProductDetails(productDetailsList.get(0))
                                    // to get an offer token, call ProductDetails.getSubscriptionOfferDetails()
                                    // for a list of offers that are available to the user
                                    //.setOfferToken(selectedOfferToken)
                                    .build()
                            );

                        final BillingFlowParams billingFlowParams = BillingFlowParams.newBuilder()
                            .setProductDetailsParamsList(productDetailsParamsList)
                            .build();

                        // Launch the billing flow
                        final BillingResult billingFlowResult = mBillingClient.launchBillingFlow(getContext(), billingFlowParams);
                    }
                }
            );
        }
        catch(Exception e) {
            Log.e(getTag(), Objects.requireNonNull(e.getMessage()));
        }
    }

    public void consumeProductItem(final String PurchaseToken, final String SKU) {
        try {
            Log.v(getTag(), "Consume: " + SKU + " " + PurchaseToken);
            mBillingClient.consumeAsync(ConsumeParams.newBuilder().setPurchaseToken(PurchaseToken).build(),
                new ConsumeResponseListener() {
                    @Override
                    public void onConsumeResponse(@NonNull BillingResult billingResult, @NonNull final String purchaseToken) {
                        try {
                            Log.v(getTag(), "onConsumeResponse Result: " + DecodeBillingResponse(billingResult.getResponseCode()) + " Token:" + purchaseToken + "; " + billingResult.getDebugMessage());
                            if (BillingResponseCode.OK == billingResult.getResponseCode()) {
                                mBillingObserver.onPurchaseConsumed(purchaseToken, SKU);
                            }
                        }
                        catch(Exception e) {
                            Log.e(getTag(), Objects.requireNonNull(e.getMessage()));
                        }
                    }
                }
            );
        }
        catch(Exception e) {
            Log.e(getTag(), Objects.requireNonNull(e.getMessage()));
        }
    }

    private void PurchaseSKUListQueried(final Purchase purchase, int BoughtOrRestored) {
        if(purchase != null) {
            final String purchToken = purchase.getPurchaseToken();
            for(String product : purchase.getProducts()) {
                if(product != null) {
                    if(!product.isEmpty()) {
                        Log.v(getTag(), "sku:" + product);
                        mBillingObserver.onPurchaseQueried(purchToken, product, BoughtOrRestored);
                    }
                }
            }
        }
    }

    private void AcknowledgePurchase(final Purchase purchase) {
        try {
            mBillingClient.acknowledgePurchase(AcknowledgePurchaseParams.newBuilder().setPurchaseToken(purchase.getPurchaseToken()).build(),
                    billingResult -> {
                        try {
                            if (BillingResponseCode.OK == billingResult.getResponseCode()) {
                                Log.v(getTag(), "onAcknowledgePurchaseResponse Result: OK");
                                PurchaseSKUListQueried(purchase, PURCHASE_BOUGHT);
                            }
                        }
                        catch(Exception e) {
                            Log.e(getTag(), Objects.requireNonNull(e.getMessage()));
                        }
                    }
            );
        }
        catch(Exception e) {
            Log.e(getTag(), Objects.requireNonNull(e.getMessage()));
        }
    }

    private void checkPurchases(BillingResult billingResult, final List<Purchase> purchases) {
        try {
            if (BillingResponseCode.OK == billingResult.getResponseCode()) {
                if (purchases != null)
                    for (Purchase purchase : purchases) {
                        if(purchase != null) {
                            switch(purchase.getPurchaseState()) {
                                case Purchase.PurchaseState.PURCHASED:
                                    if(purchase.isAcknowledged()) {
                                        Log.v(getTag(), "Purchase was Bought: " + purchase.getPurchaseToken()  + " Order:" + purchase.getOrderId());
                                        PurchaseSKUListQueried(purchase, PURCHASE_RESTORED);
                                    }
                                    else { // пробуем вызвать явно
                                        AcknowledgePurchase(purchase);
                                    }
                                    break;

                                case Purchase.PurchaseState.PENDING:
                                    Log.v(getTag(), "Purchase pending: " + purchase.getPurchaseToken());
                                    break;

                                case Purchase.PurchaseState.UNSPECIFIED_STATE:
                                    Log.v(getTag(), "Purchase unspecified_state:" + purchase.getPurchaseToken() + " Order:" + purchase.getOrderId());
                                    break;
                            }
                        }
                    }
            } else {
                Log.v(getTag(), "checkPurchases error, response = " + DecodeBillingResponse(billingResult.getResponseCode()));
            }
        }
        catch(Exception e) {
            Log.e(getTag(), Objects.requireNonNull(e.getMessage()));
        }
    }

    /////////////////////////////////////
    // PurchasesResponseListener Callbacks
    /////////////////////////////////////
    @Override
    public void onQueryPurchasesResponse(@NonNull BillingResult billingResult, @NonNull List<Purchase> purchases) {
        try {
            checkPurchases(billingResult, purchases);
        }
        catch(Exception e) {
            Log.e(getTag(), Objects.requireNonNull(e.getMessage()));
        }
    }

    /////////////////////////////////////
    // PurchasesUpdatedListener Callbacks
    /////////////////////////////////////
    @Override
    public void onPurchasesUpdated(@NonNull BillingResult billingResult, final List<Purchase> purchases) {
        try {
            checkPurchases(billingResult, purchases);
        }
        catch(Exception e) {
            Log.e(getTag(), Objects.requireNonNull(e.getMessage()));
        }
    }

    ////////////////////////////////////
    // PurchaseHistoryResponseListener Callbacks
    ////////////////////////////////////
    @Override
    public void onPurchaseHistoryResponse(@NonNull BillingResult billingResult, final List<PurchaseHistoryRecord> purchaseHistoryRecordList) {
        try {
            Log.v(getTag(), "onPurchaseHistoryResponse Result: " + DecodeBillingResponse(billingResult.getResponseCode()) + "; " + billingResult.getDebugMessage());
        }
        catch(Exception e) {
            Log.v(getTag(), Objects.requireNonNull(e.getMessage()));
        }
    }

    ////////////////////////////////////
    // BillingClientStateListener Callbacks
    ////////////////////////////////////
    @Override
    public void onBillingSetupFinished(@NonNull BillingResult billingResult) {
        try {
            final int ResponseCode = billingResult.getResponseCode();
            Log.v(getTag(), "Setup finished. Response: " + DecodeBillingResponse(ResponseCode));
            if (BillingResponseCode.OK == ResponseCode) {
                mBillingObserver.onBillingSetupFinished();
            }
            else {
                mBillingObserver.onBillingDisconnected();
            }
        }
        catch(Exception e) {
            Log.v(getTag(), Objects.requireNonNull(e.getMessage()));
        }
    }

    @Override
    public void onBillingServiceDisconnected() {
        try {
            Log.v(getTag(), "onBillingServiceDisconnected");
            mBillingObserver.onBillingDisconnected();
        }
        catch(Exception e) {
            Log.v(getTag(), Objects.requireNonNull(e.getMessage()));
        }
    }

    private String getTag() {
        return org.akkord.lib.Utils.TAG;
    }

    private Activity getContext() {
        return org.akkord.lib.Utils.GetContext();
    }
}