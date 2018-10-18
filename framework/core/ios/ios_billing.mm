#include "ios_billing.h"

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

class IosBillingStateClass
{
public:
    NSArray<SKProduct *> *products;
    SKProductsRequest *productsRequest;
};
IosBillingStateClass IosBillingState;

@interface IosBillingProdRequestDelegate<SKProductsRequestDelegate>
// ...
@end

@implementation IosBillingProdRequestDelegate
// SKProductsRequestDelegate protocol method
- (void)productsRequest:(SKProductsRequest *)request
     didReceiveResponse:(SKProductsResponse *)response
{
    IosBillingState.products = response.products;
}
@end

IosBillingProdRequestDelegate* _IosBillingProdRequestDelegate;

// API

bool iOSBillingManager::Init()
{
    _IosBillingProdRequestDelegate = [[IosBillingProdRequestDelegate alloc] init];
    return true;
};

bool iOSBillingManager::RestorePurchases()
{
    return true;
};

bool iOSBillingManager::PurchaseProdItem(const char* ProductCode)
{

    return true;
};

bool iOSBillingManager::ConsumeProductItem(const char* PurchaseToken)
{
    return true;
};

bool iOSBillingManager::QueryProductDetails(const std::vector<std::string>& ProdList)
{
    // https://eezytutorials.com/ios/nsset-by-example.php#.W8ibzWgzaHs
    NSString *values[ProdList.size()];
    for(decltype(ProdList.size()) i = 0; i < ProdList.size(); ++i)
        values[i] = [[NSString alloc] initWithUTF8String:ProdList[i].c_str()];
    
    //https://developer.apple.com/documentation/foundation/nsarray?language=objc
    NSArray *ProdsSet = [NSArray arrayWithObjects:values count:ProdList.size()];
    IosBillingState.productsRequest = [[SKProductsRequest alloc] initWithProductIdentifiers:[NSSet setWithArray:ProdsSet]];
    IosBillingState.productsRequest.delegate = _IosBillingProdRequestDelegate;
    [IosBillingState.productsRequest start];
    
    // release ProdsSet and values array
    
    return true;
};
