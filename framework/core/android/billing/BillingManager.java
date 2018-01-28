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
    
    private static BillingClient mBillingClient;
    private static int mBillingClientResponseCode = BILLING_MANAGER_NOT_INITIALIZED;
    private static boolean mIsServiceConnected = false;
    
    private static native void BillingSetupFinished(int ResponseCode);
    private static native void BillingDisconnected();
    private static native void PurchaseQueried(String PurchaseToken, String ProductSKU);
    
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
        Log.v(TAG, "startServiceConnection");
        mBillingClient.startConnection(new BillingClientStateListener() {
            @Override
            public void onBillingSetupFinished(@BillingResponse int billingResponseCode) {
                Log.d(TAG, "Setup finished. Response: " + DecodeBillingResponse(billingResponseCode));

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
    
    private static void executeServiceRequest(final Runnable runnable) {
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
    
    public static void Initialize()
    {
        Log.v(TAG, "BillingManager Initialize start");

        final Activity ctx = Utils.GetContext();
        ctx.runOnUiThread(new Runnable() {
            public void run() {        
                Log.v(TAG, "BillingManager runOnUiThread");
                mBillingClient = BillingClient.newBuilder(ctx).setListener(new PurchasesUpdatedListener()
                    {
                        @Override
                        public void onPurchasesUpdated(int responseCode, List<Purchase> purchases) 
                        {
                            // тут приходит одна покупка или несколько?
                            Log.v(TAG, "onPurchasesUpdated Response: " + DecodeBillingResponse(responseCode) + "; purch_count = " + purchases.size());
                        }                
                    }
                ).build();
                Log.v(TAG, "BillingManager after build");            
            
            
                startServiceConnection(new Runnable() {
                    @Override
                    public void run() {
                        // Notifying the listener that billing client is ready                             
                        Log.d(TAG, "Setup successful");
                        //queryPurchases();
                    }
                });    
            }
        });
    }
    
    
    public static void QueryProductDetails(String[] ProdList)
    {
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
                                //listener.onSkuDetailsResponse(responseCode, skuDetailsList);
                                Log.v(TAG, "onSkuDetailsResponse responseCode = " + responseCode + ", len = " + skuDetailsList.size());
                                for(int i = 0; i < skuDetailsList.size(); i++)
                                {
                                    SkuDetails sku = skuDetailsList.get(i);
                                    Log.v(TAG, sku.getSku() + "-" + sku.getPrice() + "-" + sku.getPriceCurrencyCode());
                                }
                            }
                        });                
            }
        });
    }
    
    public static void RestorePurchases()
    {
        executeServiceRequest(new Runnable() {
            public void run() {
                Log.v(TAG, "QueryPurchases get purchasesResult before");
                PurchasesResult purchasesResult = mBillingClient.queryPurchases(SkuType.INAPP);
                Log.v(TAG, "QueryPurchases ResponseCode = " + purchasesResult.getResponseCode());
                
                //https://developer.android.com/reference/com/android/billingclient/api/Purchase.html
                
                if(BillingResponse.OK == purchasesResult.getResponseCode())
                {            
                    List<Purchase> purchases = purchasesResult.getPurchasesList();            
                    for (Purchase purchase : purchases) {
                        //handlePurchase(purchase);
                        //Log.v(TAG, "QueryPurchases: " + purchase.getSku());
                        PurchaseQueried(purchase.getPurchaseToken(), purchase.getSku());
                    }
                }
                else
                {
                    Log.v(TAG, "QueryPurchases error, responseCode = " + purchasesResult.getResponseCode());
                }
            }
        });
    }
    
    public static void PurchaseProdItem(final String ProductSKU)
    {
        executeServiceRequest(new Runnable() {
            public void run() {        
                BillingFlowParams purchaseParams = BillingFlowParams.newBuilder().setSku(ProductSKU).setType(SkuType.INAPP)/*.setOldSkus(oldSkus))*/.build();
                mBillingClient.launchBillingFlow(Utils.GetContext(), purchaseParams);
            }
        });
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
        
        executeServiceRequest(new Runnable() {
            public void run() {        
                Log.v(TAG, "ConsumeProductItem executeServiceRequest");
                mBillingClient.consumeAsync(PurchaseToken, null);
            }
        });
    }    
}