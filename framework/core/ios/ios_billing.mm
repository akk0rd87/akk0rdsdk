#include "ios_billing.h"

#import <StoreKit/StoreKit.h>

class IosBillingStateClass
{
public:
    SKMutablePayment *payment;
    void GetProdDetails()
    {

        
    };
};
static IosBillingStateClass IosBillingState;

bool iOSBillingManager::Init()
{
    return true;
};

bool iOSBillingManager::RestorePurchases()
{
    return true;
};

bool iOSBillingManager::PurchaseProdItem(const char* ProductCode)
{
    SKProduct *product;
    IosBillingState.payment = [SKMutablePayment paymentWithProduct:product];
    IosBillingState.payment.quantity = 1;
    
    [[SKPaymentQueue defaultQueue] addPayment:IosBillingState.payment];
    return true;
};

bool iOSBillingManager::ConsumeProductItem(const char* PurchaseToken)
{
    return true;
};

bool iOSBillingManager::QueryProductDetails(const std::vector<std::string>& ProdList)
{
    //SKProductsRequest *productsRequest = [[SKProductsRequest alloc]
        //                                  initWithProductIdentifiers:[NSSet setWithArray:productIdentifiers]];
    
    // Keep a strong reference to the request.
    //self.request = productsRequest;
    //productsRequest.delegate = self;
    //[productsRequest start];
    
    
    return true;
};
