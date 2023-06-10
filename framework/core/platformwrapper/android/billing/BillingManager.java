package org.akkord.lib;

import android.app.Activity;
import android.content.Context;

import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.BillingClient.BillingResponseCode;
import com.android.billingclient.api.BillingClient.FeatureType;
import com.android.billingclient.api.BillingClient.ProductType;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.PurchasesResponseListener;
import com.android.billingclient.api.ProductDetailsResponseListener;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.ConsumeResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.Purchase.PurchaseState;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.SkuDetails;
import com.android.billingclient.api.SkuDetailsParams;
import com.android.billingclient.api.SkuDetailsResponseListener;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.ConsumeParams.Builder;
import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.AcknowledgePurchaseResponseListener;
import com.android.billingclient.api.PurchaseHistoryRecord;
import com.android.billingclient.api.PurchaseHistoryResponseListener;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.TimeUnit;
import android.util.Log;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.Arrays;
import java.util.HashMap;

public class BillingManager implements PurchaseHistoryResponseListener, PurchasesUpdatedListener, BillingClientStateListener, PurchasesResponseListener, ProductDetailsResponseListener {
    private static BillingManager billingManager = new BillingManager();
    private static BlockingQueue<Runnable> runnableQueue = new ArrayBlockingQueue<Runnable>(50, true);

    private final static String TAG = "SDL";
    public static final int BILLING_MANAGER_NOT_INITIALIZED  = -1;

    public static final int PURCHASE_RESTORED = 0;
    public static final int PURCHASE_BOUGHT   = 1;

    private static BillingClient mBillingClient = null;

    private static AtomicBoolean billingClientLocked = new AtomicBoolean(false);

    private static native void BillingSetupFinished(int ResponseCode);
    private static native void BillingDisconnected();
    private static native void PurchaseQueried(String PurchaseToken, String ProductSKU, int Type);
    private static native void PurchaseConsumed(String PurchaseToken, String ProductSKU);

    private static HashMap<String, ProductDetails> CachedSKUMap = new HashMap<>(); // кешированный список товаров

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

    private static void checkBillingConnection() {
        try {
            if(mBillingClient.isReady()) {
                return;
            }

            if(billingClientLocked.compareAndSet(false, true)) {
                mBillingClient.startConnection(billingManager);
            };
        }
        catch(Exception e) {
            Log.v(TAG, e.getMessage());
        }
    }

    public static void Initialize()
    {
        try {
            Log.v(TAG, "BillingManager Initialize start");
            mBillingClient = BillingClient.newBuilder(Utils.GetContext())
            .setListener(billingManager) // PurchasesUpdatedListener
            .enablePendingPurchases()
            .build();
            Log.v(TAG, "BillingManager after build");
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }

    private static void QueryProductDetails_v5(final String[] ProdList) {
        // this method uses deprecated API
        try {
            Log.v(TAG, "QueryProductDetails_v5 before");
            final List<String> skuList = Arrays.asList(ProdList);
            executeServiceRequest(new Runnable() {
                public void run() {
                    try {
                        // Query the purchase async
                        // Product id must be provided.
                        final List<QueryProductDetailsParams.Product> productList = List.of(QueryProductDetailsParams.Product.newBuilder()
                                                                    .setProductType(ProductType.INAPP)
                                                                    .build());

                        QueryProductDetailsParams params = QueryProductDetailsParams.newBuilder()
                            .setProductList(productList)
                            .build();

                        mBillingClient.queryProductDetailsAsync(params, billingManager);
                    }
                    catch(Exception e) {
                        Log.v(TAG, e.getMessage());
                    }
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void QueryProductDetails(final String[] ProdList)
    {
        try {
            //QueryProductDetails_v5();
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void RestorePurchases()
    {
        try {
            Log.v(TAG, "public static void RestorePurchases()");
            executeServiceRequest(new Runnable() {
                public void run() {
                    try {
                        Log.v(TAG, "QueryPurchases get purchasesResult before");
                        mBillingClient.queryPurchasesAsync(ProductType.INAPP, billingManager);
                    }
                    catch(Exception e)  {
                        Log.e(TAG, e.getMessage());
                    }
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void PurchaseProdItem(final String ProductSKU)
    {
        try {
            final List<QueryProductDetailsParams.Product> productList = List.of(QueryProductDetailsParams.Product.newBuilder()
                                                        .setProductType(ProductType.INAPP)
                                                        .setProductId(ProductSKU)
                                                        .build());

            QueryProductDetailsParams params = QueryProductDetailsParams.newBuilder()
                .setProductList(productList)
                .build();

            mBillingClient.queryProductDetailsAsync(params,
                new ProductDetailsResponseListener() {
                    public void onProductDetailsResponse(BillingResult billingResult, final List<ProductDetails> productDetailsList) {
                        if( BillingResponseCode.OK == billingResult.getResponseCode() && productDetailsList != null && !productDetailsList.isEmpty() ) {

                            List productDetailsParamsList =
                                List.of(
                                    BillingFlowParams.ProductDetailsParams.newBuilder()
                                        .setProductDetails(productDetailsList.get(0))
                                        // to get an offer token, call ProductDetails.getSubscriptionOfferDetails()
                                        // for a list of offers that are available to the user
                                        //.setOfferToken(selectedOfferToken)
                                        .build()
                                );

                            BillingFlowParams billingFlowParams = BillingFlowParams.newBuilder()
                                .setProductDetailsParamsList(productDetailsParamsList)
                                .build();

                            // Launch the billing flow
                            BillingResult billingFlowResult = mBillingClient.launchBillingFlow(Utils.GetContext(), billingFlowParams);
                        }
                    }
                }
            );


/*
            // делаем динамический запрос
            final List<String> skuList = Arrays.asList(new String[]{ProductSKU});
            executeServiceRequest(new Runnable() {
                public void run() {
                    try {
                        // Query the skuList async
                        Log.d(TAG, "Query the skuList async: " + ProductSKU);
                        SkuDetailsParams.Builder params = SkuDetailsParams.newBuilder();
                        params.setSkusList(skuList).setType(ProductType.INAPP);
                        mBillingClient.querySkuDetailsAsync(params.build(),
                            new SkuDetailsResponseListener() {
                                @Override
                                public void onSkuDetailsResponse(BillingResult billingResult, final List<SkuDetails> skuDetailsList) {
                                    try {
                                        if (BillingResponseCode.OK == billingResult.getResponseCode()) {
                                            if (skuDetailsList != null) {
                                                if(skuDetailsList.size() == 1) {
                                                    for (SkuDetails skuDetails : skuDetailsList) {
                                                        mBillingClient.launchBillingFlow(
                                                            Utils.GetContext(),
                                                            BillingFlowParams.newBuilder().setSkuDetails(skuDetails).build()
                                                        );
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    catch(Exception e) {
                                        Log.e(TAG, e.getMessage());
                                    }
                                };
                            }
                        );
                    }
                    catch(Exception e) {
                        Log.e(TAG, e.getMessage());
                    }
                }
            });
*/

        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void ConsumeProductItem(final String PurchaseToken, final String SKU)
    {
        try {
            executeServiceRequest(new Runnable() {
                public void run() {
                    try {
                        Log.v(TAG, "Consume: " + SKU + " " + PurchaseToken);
                        mBillingClient.consumeAsync(ConsumeParams.newBuilder().setPurchaseToken(PurchaseToken).build(),
                            new ConsumeResponseListener() {
                                @Override
                                public void onConsumeResponse(BillingResult billingResult, final String purchaseToken) {
                                    try {
                                        Log.v(TAG, "onConsumeResponse Result: " + DecodeBillingResponse(billingResult.getResponseCode()) + " Token:" + purchaseToken + "; " + billingResult.getDebugMessage());
                                        if (BillingResponseCode.OK == billingResult.getResponseCode()) {
                                            PurchaseConsumed(purchaseToken, SKU);
                                        }
                                    }
                                    catch(Exception e) {
                                        Log.e(TAG, e.getMessage());
                                    }
                                }
                            }
                        );
                    }
                    catch(Exception e) {
                        Log.e(TAG, e.getMessage());
                    }
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }

    private static void PurchaseSKUListQueried(final Purchase purchase, int BoughtOrRestored) {
        if(purchase != null) {
            if(purchase.getProducts() != null) {
                final String purchToken = purchase.getPurchaseToken();
                for(String product : purchase.getProducts()) {
                    if(product != null) {
                        if(!product.isEmpty()) {
                            Log.v(TAG, "sku:" + product);
                            PurchaseQueried(purchToken, product, BoughtOrRestored);
                        }
                    }
                }
            }
        }
    }

    private static void AcknowledgePurchase(final Purchase purchase) {
        try {
            executeServiceRequest(new Runnable() {
                public void run() {
                    try {
                        mBillingClient.acknowledgePurchase(AcknowledgePurchaseParams.newBuilder().setPurchaseToken(purchase.getPurchaseToken()).build(),
                            new AcknowledgePurchaseResponseListener() {
                                @Override
                                public void onAcknowledgePurchaseResponse(BillingResult billingResult) {
                                    try {
                                        if (BillingResponseCode.OK == billingResult.getResponseCode()) {
                                            Log.v(TAG, "onAcknowledgePurchaseResponse Result: " + DecodeBillingResponse(billingResult.getResponseCode()) + "; " + billingResult.getDebugMessage());
                                            PurchaseSKUListQueried(purchase, PURCHASE_BOUGHT);
                                        }
                                    }
                                    catch(Exception e) {
                                        Log.e(TAG, e.getMessage());
                                    }
                                };
                            }
                        );
                    }
                    catch(Exception e) {
                        Log.e(TAG, e.getMessage());
                    }
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }

    public static void QueryPurchaseHistory() {
        try {
            executeServiceRequest(new Runnable() {
                public void run() {
                    try {
                        mBillingClient.queryPurchaseHistoryAsync(ProductType.INAPP, billingManager);
                    }
                    catch(Exception e) {
                        Log.e(TAG, e.getMessage());
                    }
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }

    private static void processRunnableQueue() {
        try {
            if((runnableQueue.size() > 0) && mBillingClient.isReady()) {
                Runnable r = new Runnable() {
                    public void run() {
                        try {
                            // тут тоже можно поставить concurrent проверку, специфичную только для этого процесса
                            while(runnableQueue.size() > 0 && mBillingClient.isReady()) {
                                try {
                                    Runnable rTask = runnableQueue.take();
                                    rTask.run();
                                }
                                catch(Exception e) {
                                    Log.e(TAG, e.getMessage());
                                }
                            };
                        }
                        catch(Exception e) {
                            Log.e(TAG, e.getMessage());
                        }
                    };
                };
                r.run();
            }
        }
        catch(Exception e) {
            Log.v(TAG, e.getMessage());
        }
    }

    private static void executeServiceRequest(final Runnable R) {
        try{
            Log.v(TAG, "starting executeServiceRequest");
            runnableQueue.add(R);
            if(mBillingClient.isReady()) {
                processRunnableQueue();
            }
            else {
                checkBillingConnection();
            }
            Log.v(TAG, "after executeServiceRequest");
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
            e.printStackTrace();
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
                                        Log.v(TAG, "Purchase was Bought: " + purchase.getPurchaseToken()  + " Order:" + purchase.getOrderId());
                                        PurchaseSKUListQueried(purchase, PURCHASE_RESTORED);
                                    }
                                    else { // пробуем вызвать явно
                                        AcknowledgePurchase(purchase);
                                    }
                                    break;

                                case Purchase.PurchaseState.PENDING:
                                    Log.v(TAG, "Purchase pending: " + purchase.getPurchaseToken());
                                    break;

                                case Purchase.PurchaseState.UNSPECIFIED_STATE:
                                    Log.v(TAG, "Purchase unspecified_state:" + purchase.getPurchaseToken() + " Order:" + purchase.getOrderId());
                                    break;
                            }
                        }
                    }
            } else {
                Log.v(TAG, "checkPurchases error, response = " + DecodeBillingResponse(billingResult.getResponseCode()));
            }
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
            e.printStackTrace();
        }
    }

    /////////////////////////////////////
    // PurchasesResponseListener Callbacks
    /////////////////////////////////////
    @Override
    public void onQueryPurchasesResponse(BillingResult billingResult, List<Purchase> purchases) {
        try {
            checkPurchases(billingResult, purchases);
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
            e.printStackTrace();
        }
    }

    /////////////////////////////////////
    // PurchasesUpdatedListener Callbacks
    /////////////////////////////////////
    @Override
    public void onPurchasesUpdated(BillingResult billingResult, final List<Purchase> purchases) {
        try {
            checkPurchases(billingResult, purchases);
        }
        catch(Exception e) {
            Log.e(TAG, e.getMessage());
            e.printStackTrace();
        }
    }

/*
    ///////////////////////////////////////
    // SkuDetailsResponseListener Callbacks
    ///////////////////////////////////////
    @Override // deprecated API
    public void onSkuDetailsResponse(BillingResult billingResult, final List<SkuDetails> skuDetailsList) {
        try {
            //Log.v(TAG, "onSkuDetailsResponse Response = " + DecodeBillingResponse(billingResult.getResponseCode()));
            //if (BillingResponseCode.OK == billingResult.getResponseCode()) {
            //    if (skuDetailsList != null)
            //        for (SkuDetails skuDetails : skuDetailsList) { // кешируем список товаров
            //            CachedSKUMap.put(skuDetails.getSku(), skuDetails);
            //        }
            //
            //    if(true) { // logging
            //        for(HashMap.Entry<String, SkuDetails> entry : CachedSKUMap.entrySet())
            //            Log.v(TAG, "cache SKU ID: " + entry.getKey());
            //    }
            //}
        }
        catch(Exception e) {
            Log.v(TAG, e.getMessage());
        }
    }
*/

    ///////////////////////////////////////
    // ProductDetailsResponseListener Callbacks
    ///////////////////////////////////////
    @Override
    public void onProductDetailsResponse(BillingResult billingResult, List<ProductDetails> productDetailsList) {
        try {
            Log.v(TAG, "onProductDetailsResponse Response = " + DecodeBillingResponse(billingResult.getResponseCode()));
            if (BillingResponseCode.OK == billingResult.getResponseCode()) {
                if (productDetailsList != null)
                    for (ProductDetails productDetails : productDetailsList) { // кешируем список товаров
                        if(productDetails != null) {
                            Log.v(TAG, "product details: sku=" + productDetails.getProductId());
                            CachedSKUMap.put(productDetails.getProductId(), productDetails);
                        }
                    }

                if(true) { // logging
                    for(HashMap.Entry<String, ProductDetails> entry : CachedSKUMap.entrySet())
                        Log.v(TAG, "cache SKU ID: " + entry.getKey());
                }
            }
        }
        catch(Exception e) {
            Log.v(TAG, e.getMessage());
        }
    }

    ////////////////////////////////////
    // PurchaseHistoryResponseListener Callbacks
    ////////////////////////////////////
    @Override
    public void onPurchaseHistoryResponse(BillingResult billingResult, final List<PurchaseHistoryRecord> purchaseHistoryRecordList) {
        try {
            Log.v(TAG, "onPurchaseHistoryResponse Result: " + DecodeBillingResponse(billingResult.getResponseCode()) + "; " + billingResult.getDebugMessage());
        }
        catch(Exception e) {
            Log.v(TAG, e.getMessage());
        }
    }

    ////////////////////////////////////
    // BillingClientStateListener Callbacks
    ////////////////////////////////////
    @Override
    public void onBillingSetupFinished(BillingResult billingResult) {
        try {
            Log.v(TAG, "Setup finished. Response: " + DecodeBillingResponse(billingResult.getResponseCode()));
            BillingSetupFinished(billingResult.getResponseCode());
            billingClientLocked.set(false);

            if(runnableQueue.size() > 0) { // тут нужно отпроцессить очередь
                processRunnableQueue();
            }
        }
        catch(Exception e) {
            Log.v(TAG, e.getMessage());
        }
    }

    @Override
    public void onBillingServiceDisconnected() {
        try {
            Log.v(TAG, "onBillingServiceDisconnected");
            BillingDisconnected();
            billingClientLocked.set(false);

            // если очередь непуста, то делаем рестар connection
            if(runnableQueue.size() > 0) {
                checkBillingConnection();
            }
        }
        catch(Exception e) {
            Log.v(TAG, e.getMessage());
        }
    }
}