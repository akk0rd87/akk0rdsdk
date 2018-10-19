#include "ios_billing.h"

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

@interface FSProductStore : NSObject
+ (FSProductStore *)defaultStore;

// AppDelegate will call this on app start ...
- (void)registerObserver;
// handling product requests ...
- (void)startProductRequestWithIdentifier:(const std::vector<std::string>&)productIdentifiers /*completionHandler:(void (^)(BOOL success, NSError *error))completionHandler*/;
- (void)purshaseProdItem:(const char*) ProdItem;
- (void)cancelProductRequest;
@end

@interface FSProductStore () <SKProductsRequestDelegate, SKPaymentTransactionObserver>
- (void)startTransaction:(SKPaymentTransaction *)transaction;
- (void)completeTransaction:(SKPaymentTransaction *)transaction;
- (void)failedTransaction:(SKPaymentTransaction *)transaction;
- (void)restoreTransaction:(SKPaymentTransaction *)transaction;
- (void)validateReceipt:(NSData *)receiptData withCompletionHandler:(void (^)(BOOL success, NSError *error))completionHandler;
- (void)purchaseSuccess:(NSString *)productIdentifier;
- (void)purchaseFailedWithError:(NSError *)error;
@property (nonatomic, strong) SKProductsRequest *currentProductRequest;
@property (nonatomic, copy) void (^completionHandler)(BOOL success, NSError *error);
@property (nonatomic, strong) NSArray<SKProduct *> *products;
@end

@implementation FSProductStore

+ (FSProductStore *)defaultStore
{
    static FSProductStore *store;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (!store)
        {
            store = [[FSProductStore alloc] init];
        }
    });
    return store;
}

- (void)registerObserver
{
    logDebug("Registering observer ...");
    [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
}

#pragma mark - Products request delegate
- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
    if (!response.products || response.products.count == 0)
    {
        self.products = nullptr;
        logError("Zero products received");
    }
    else
    {
        logDebug("Products requested %d", response.products.count);
        self.products = response.products;
    }
}

- (void)purshaseProdItem:(const char*) ProdItem
{
    NSString *ProdID = [[NSString alloc] initWithUTF8String:ProdItem];
    
    if(self.products != nullptr)
    {
        for(decltype(self.products.count) i = 0; i < self.products.count; ++ i)
        {
            if(self.products[i] != nullptr)
            {
                if([self.products[i].productIdentifier isEqualToString:ProdID])
                {
                    SKPayment *payment = [SKPayment paymentWithProduct:self.products[i]];
                    if ([SKPaymentQueue canMakePayments])
                    {
                        [[SKPaymentQueue defaultQueue] addPayment:payment];
                    }
                    else
                    {
                        logError("Payment add error");
                    }
                    // go to [ProdID release];
                    break;
                }
            }
            else
            {
                logError((std::string("Product ") + std::to_string(i) + " is empty").c_str());
            }
        }
        
        logError("Prod ID %s not found", ProdItem);
    }
    else
    {
        logError("Prod list is empty");
    }

    [ProdID release];
}

#pragma mark - Payment transaction observer
// Sent when the transaction array has changed (additions or state changes).  Client should check state of transactions and finish as appropriate.
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions
{
    for (SKPaymentTransaction *transaction in transactions)
    {
        switch (transaction.transactionState)
        {
            case SKPaymentTransactionStatePurchasing: [self startTransaction:transaction];    break;
            case SKPaymentTransactionStateFailed:     [self failedTransaction:transaction];   break;
            case SKPaymentTransactionStatePurchased:  [self completeTransaction:transaction]; break;
            case SKPaymentTransactionStateRestored:   [self restoreTransaction:transaction];  break;
            default: break;
        }
    }
}

// Sent when an error is encountered while adding transactions from the user's purchase history back to the queue.
- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error
{
    //DLog(@"%@", error);
    [self purchaseFailedWithError:error];
}

// Sent when all transactions from the user's purchase history have successfully been added back to the queue.
- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue
{
    //DLog(@"%@", queue);
}

// Sent when the download state has changed.
- (void)paymentQueue:(SKPaymentQueue *)queue updatedDownloads:(NSArray *)downloads
{
    //DLog(@"%@", downloads);
}

- (void)startProductRequestWithIdentifier:(const std::vector<std::string>&)productIdentifiers
                        /*completionHandler:(void (^)(BOOL success, NSError *error))completionHandler*/
{
    // cancel any existing product request (if exists) ...
    [self cancelProductRequest];
    
    // start new  request ...
    //self.completionHandler = completionHandler;
    NSString *values[productIdentifiers.size()];
    
    for(decltype(productIdentifiers.size()) i = 0; i < productIdentifiers.size(); ++i)
        values[i] = [[NSString alloc] initWithUTF8String:productIdentifiers[i].c_str()];
    
    NSArray *ProdsSet = [NSArray arrayWithObjects:values count:productIdentifiers.size()];
    self.currentProductRequest  = [[SKProductsRequest alloc] initWithProductIdentifiers:[NSSet setWithArray:ProdsSet]];

    _currentProductRequest.delegate = self;
    [_currentProductRequest start];

    for(auto& v : values)
        [v release];
}

- (void)cancelProductRequest
{
    if (_currentProductRequest)
    {
        //DLog(@"cancelling existing request ...");
        
        [_currentProductRequest setDelegate:nil];
        [_currentProductRequest cancel];
    }
}

- (void)startTransaction:(SKPaymentTransaction *)transaction
{
    //DLog(@"starting transaction: %@", transaction);
}

- (void)completeTransaction: (SKPaymentTransaction *)transaction
{
    [self validateReceipt:transaction.transactionReceipt withCompletionHandler:^ (BOOL success, NSError *error) {
        if (success)
        {
            //[self recordTransaction:transaction];
            //[self purchaseSuccess:transaction.payment.productIdentifier];
            
            //[self onPurchaseCompleted:transaction];
        }
        else
        {
            // deal with error ...
            //[self purchaseFailedWithError:error];
        }
        
        [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
    }];
}

- (void)failedTransaction: (SKPaymentTransaction *)transaction
{
    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
    
    if (transaction.error.code != SKErrorPaymentCancelled) {
        [self purchaseFailedWithError:transaction.error];
    }
    else
    {
        [self purchaseFailedWithError:nil];
    }
}

- (void)restoreTransaction: (SKPaymentTransaction *)transaction
{
    [self recordTransaction:transaction];
    [self purchaseSuccess:transaction.originalTransaction.payment.productIdentifier];
    
    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
}

- (void)recordTransaction:(SKPaymentTransaction *)transaction
{
    // TODO: store for audit trail - perhaps on remote server?
    //FSTransaction *transactionToRecord = [FSTransaction transactionWithIdentifier:transaction.transactionIdentifier receipt:transaction.transactionReceipt];
    //[transactionToRecord store];
}

- (void)purchaseSuccess:(NSString *)productIdentifier
{
    // TODO: make purchase available to user - perhaps call completion block?
    
    self.currentProductRequest = nil;
    
    if (_completionHandler)
    {
        _completionHandler(YES, nil);
    }
}

- (void)purchaseFailedWithError:(NSError *)error
{
    self.currentProductRequest = nil;
    
    if (_completionHandler)
    {
        _completionHandler(NO, error);
    }
}

- (void)validateReceipt:(NSData *)receiptData withCompletionHandler:(void (^)(BOOL success, NSError *error))completionHandler
{
    // BODY OF THIS METHOD WAS DELETED
}

- (void)onPurchaseCompleted:(SKPaymentTransaction *)transaction
{
    if (transaction.originalTransaction != nil)
    {
        return; // existing transaction ...
    }
    
    // THIS PART OF THIS METHOD WAS DELETED
}
@end


//
// API
//

bool iOSBillingManager::Init()
{
   // _IosBillingProdRequestDelegate = [[IosBillingProdRequestDelegate alloc] init];
    [[FSProductStore defaultStore] registerObserver];
    
    return true;
};

bool iOSBillingManager::RestorePurchases()
{
    return true;
};

bool iOSBillingManager::PurchaseProdItem(const char* ProductCode)
{
    [[FSProductStore defaultStore] purshaseProdItem:ProductCode];
    return true;
};

bool iOSBillingManager::ConsumeProductItem(const char* PurchaseToken)
{
    return true;
};

bool iOSBillingManager::QueryProductDetails(const std::vector<std::string>& ProdList)
{
    [[FSProductStore defaultStore] startProductRequestWithIdentifier:ProdList];
    
    return true;
};
