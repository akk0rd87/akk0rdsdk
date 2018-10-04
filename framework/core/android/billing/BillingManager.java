package org.akkord.lib;

import android.app.Activity;
import android.content.Context;

import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClient.BillingResponse;
import com.android.billingclient.api.BillingClient.FeatureType;
import com.android.billingclient.api.BillingClient.SkuType;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.ConsumeResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.Purchase.PurchasesResult;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.SkuDetails;
import com.android.billingclient.api.SkuDetailsParams;
import com.android.billingclient.api.SkuDetailsResponseListener;
import android.util.Log;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.Arrays;


class BillingManager {
    private static String TAG = "SDL";
    public static final int BILLING_MANAGER_NOT_INITIALIZED  = -1;

    public static final int PURCHASE_RESTORED = 0;
    public static final int PURCHASE_BOUGHT   = 1;
    
    private static BillingClient mBillingClient;
    private static int mBillingClientResponseCode = BILLING_MANAGER_NOT_INITIALIZED;
    private static boolean mIsServiceConnected = false;
    
    private static native void BillingSetupFinished(int ResponseCode);
    private static native void BillingDisconnected();
    private static native void PurchaseQueried(String PurchaseToken, String ProductSKU, int Type);
    private static native void PurchaseConsumed(String PurchaseToken);
    
    private static String DecodeBillingResponse(int billingResponseCode)
    {
        String StringCode = "UNKNOWN";
        switch(billingResponseCode)
        {
            case BillingResponse.BILLING_UNAVAILABLE: StringCode = "BILLING_UNAVAILABLE"; break;
            case BillingResponse.DEVELOPER_ERROR: StringCode = "DEVELOPER_ERROR"; break;
            case BillingResponse.ERROR: StringCode = "ERROR"; break;
            case BillingResponse.FEATURE_NOT_SUPPORTED: StringCode = "FEATURE_NOT_SUPPORTED"; break;
            case BillingResponse.ITEM_ALREADY_OWNED: StringCode = "ITEM_ALREADY_OWNED"; break;
            case BillingResponse.ITEM_NOT_OWNED: StringCode = "ITEM_NOT_OWNED"; break;
            case BillingResponse.ITEM_UNAVAILABLE: StringCode = "ITEM_UNAVAILABLE"; break;
            case BillingResponse.OK: StringCode = "OK"; break;
            case BillingResponse.SERVICE_DISCONNECTED: StringCode = "SERVICE_DISCONNECTED"; break;
            case BillingResponse.SERVICE_UNAVAILABLE: StringCode = "SERVICE_UNAVAILABLE"; break;
            case BillingResponse.USER_CANCELED: StringCode = "USER_CANCELED"; break;                    
        }
        
        return StringCode;
    }
    
    private static void startServiceConnection(final Runnable executeOnSuccess) {
        try {
            Log.v(TAG, "startServiceConnection");
            mBillingClient.startConnection(new BillingClientStateListener() {
                @Override
                public void onBillingSetupFinished(@BillingResponse int billingResponseCode) {
                    Log.v(TAG, "Setup finished. Response: " + DecodeBillingResponse(billingResponseCode));

                    if (billingResponseCode == BillingResponse.OK) {
                        mIsServiceConnected = true;
                        if (executeOnSuccess != null) {
                            executeOnSuccess.run();
                        }
                    }
                    mBillingClientResponseCode = billingResponseCode;
                    BillingSetupFinished(billingResponseCode);
                }

                @Override
                public void onBillingServiceDisconnected() {
                    Log.v(TAG, "onBillingServiceDisconnected");
                    mIsServiceConnected = false;
                    BillingDisconnected();
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }
    
    private static void executeServiceRequest(final Runnable runnable) {
        try {
            Activity ctx = Utils.GetContext();
            ctx.runOnUiThread(new Runnable() {
                public void run() {
                    if (mIsServiceConnected) {
                        runnable.run();
                    } else {
                        // If billing service was disconnected, we try to reconnect 1 time.
                        // (feel free to introduce your retry policy here).
                        startServiceConnection(runnable);
                    }
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }    
    
    public static void Initialize()
    {
        try {
            Log.v(TAG, "BillingManager Initialize start");

            final Activity ctx = Utils.GetContext();
            ctx.runOnUiThread(new Runnable() {
                public void run() {
                    Log.v(TAG, "BillingManager runOnUiThread");
                    mBillingClient = BillingClient.newBuilder(ctx).setListener(new PurchasesUpdatedListener() {
                                                                                   @Override
                                                                                   public void onPurchasesUpdated(int responseCode, List<Purchase> purchases) {
                                                                                       // тут приходит одна покупка или несколько?

                                                                                       if (BillingResponse.OK == responseCode) {
                                                                                           if (purchases != null)
                                                                                               for (Purchase purchase : purchases) {
                                                                                                   Log.v(TAG, "Purchase was Bought = " + purchase.getPurchaseToken() + " " + purchase.getSku() + "Order:" + purchase.getOrderId());
                                                                                                   PurchaseQueried(purchase.getPurchaseToken(), purchase.getSku(), PURCHASE_BOUGHT);
                                                                                               }
                                                                                       } else {
                                                                                           Log.v(TAG, "QueryPurchases error, response = " + DecodeBillingResponse(responseCode));
                                                                                       }

                                                                                   }
                                                                               }
                    ).build();
                    Log.v(TAG, "BillingManager after build");
            
                /*
				startServiceConnection(new Runnable() {
                    @Override
                    public void run() {
                        // Notifying the listener that billing client is ready                             
                        Log.v(TAG, "Setup successful");
                        //queryPurchases();
                    }
                });    
				*/
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }
    
    
    public static void QueryProductDetails(String[] ProdList)
    {
        try {
            // https://stackoverflow.com/questions/19591873/get-an-array-of-strings-from-java-to-c-jni

            final List<String> skuList = Arrays.asList(ProdList);
            Log.v(TAG, "GetProductDetails " + skuList.get(1) + " as product");

            //Activity ctx = Utils.GetContext();
            executeServiceRequest(new Runnable() {
                public void run() {
                    // Query the purchase async
                    SkuDetailsParams.Builder params = SkuDetailsParams.newBuilder();
                    params.setSkusList(skuList).setType(SkuType.INAPP);
                    mBillingClient.querySkuDetailsAsync(params.build(),
                            new SkuDetailsResponseListener() {
                                @Override
                                public void onSkuDetailsResponse(int responseCode,
                                                                 List<SkuDetails> skuDetailsList) {
                                    if (BillingResponse.OK == responseCode)
                                        if (skuDetailsList != null)
                                            for (int i = 0; i < skuDetailsList.size(); i++) {
                                                // Need to set Java Native Callback
                                                //SkuDetails sku = skuDetailsList.get(i);
                                                //Log.v(TAG, sku.getSku() + "-" + sku.getPrice() + "-" + sku.getPriceCurrencyCode());
                                            }
                                }
                            });
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }
    
    public static void RestorePurchases()
    {
        try {
            executeServiceRequest(new Runnable() {
                public void run() {
                    Log.v(TAG, "QueryPurchases get purchasesResult before");
                    PurchasesResult purchasesResult = mBillingClient.queryPurchases(SkuType.INAPP);
                    Log.v(TAG, "QueryPurchases Response = " + DecodeBillingResponse(purchasesResult.getResponseCode()));

                    //https://developer.android.com/reference/com/android/billingclient/api/Purchase.html

                    if (BillingResponse.OK == purchasesResult.getResponseCode()) {
                        List<Purchase> purchases = purchasesResult.getPurchasesList();
                        for (Purchase purchase : purchases) {

                            Log.v(TAG, "Purchase was Restored = " + purchase.getPurchaseToken() + " " + purchase.getSku() + "Order:" + purchase.getOrderId());
                            PurchaseQueried(purchase.getPurchaseToken(), purchase.getSku(), PURCHASE_RESTORED);
                        }
                    } else {
                        Log.v(TAG, "QueryPurchases error, response = " + DecodeBillingResponse(purchasesResult.getResponseCode()));
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
            executeServiceRequest(new Runnable() {
                public void run() {
                    BillingFlowParams purchaseParams = BillingFlowParams.newBuilder().setSku(ProductSKU).setType(SkuType.INAPP)/*.setOldSkus(oldSkus))*/.build();
                    mBillingClient.launchBillingFlow(Utils.GetContext(), purchaseParams);
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }
    
    public static void ConsumeProductItem(final String PurchaseToken)
    {
        /*
        Log.v(TAG, "ConsumeProductItem started");
        // Generating Consume Response listener
        final ConsumeResponseListener onConsumeListener = new ConsumeResponseListener() {
            @Override
            public void onConsumeResponse(@BillingResponse int responseCode, String purchaseToken) {
                Log.v(TAG, "onConsumeResponse");
                Log.v(TAG, "onConsumeResponse code = " + responseCode + " token = " + purchaseToken);
            }
        };
        */        
        try {
            executeServiceRequest(new Runnable() {
                public void run() {
                    Log.v(TAG, "ConsumeResponse: " + PurchaseToken);
                    mBillingClient.consumeAsync(PurchaseToken, new ConsumeResponseListener() {
                        @Override
                        public void onConsumeResponse(int responseCode, String purchaseToken) {
                            Log.v(TAG, "onConsumeResponse Result: " + DecodeBillingResponse(responseCode) + " Token:" + PurchaseToken);
                            if (responseCode == BillingResponse.OK) {
                                PurchaseConsumed(purchaseToken);
                            }
                        }
                    });
                }
            });
        }
        catch(Exception e)
        {
            Log.e(TAG, e.getMessage());
        }
    }    
}