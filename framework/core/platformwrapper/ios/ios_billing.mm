#include "ios_billing.h"

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

class IosBillingStateClass
{
public:
    BillingManager::BillingCallbackObserver callbackObserver;
    IosBillingStateClass() : callbackObserver(nullptr) {}
};
static IosBillingStateClass IosBillingState;

@interface FSProductStore : NSObject
+ (FSProductStore *)defaultStore;

// AppDelegate will call this on app start ...
- (void)registerObserver;
// handling product requests ...
- (void)startProductRequestWithIdentifier:(const std::vector<std::string>&)productIdentifiers /*completionHandler:(void (^)(BOOL success, NSError *error))completionHandler*/;
- (void)purshaseProdItem:(const char*) ProdItem;
- (void)restoreTransactions;
- (void)cancelProductRequest;
@end

@interface FSProductStore () <SKProductsRequestDelegate, SKPaymentTransactionObserver>
- (void)startTransaction:(SKPaymentTransaction *)transaction;
- (void)completeTransaction:(SKPaymentTransaction *)transaction;
- (void)restoreTransaction:(SKPaymentTransaction *)transaction;
- (void)validateReceipt:(NSData *)receiptData withCompletionHandler:(void (^)(BOOL success, NSError *error))completionHandler;
@property (nonatomic, strong) SKProductsRequest *currentProductRequest;
@property (nonatomic, copy) void (^completionHandler)(BOOL success, NSError *error);
@property (nonatomic, retain) NSArray<SKProduct *> *products;
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
        for(decltype(self.products.count) i = 0; i < self.products.count; ++i)
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
    //[self purchaseFailedWithError:error];
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

- (void)restoreTransactions
{
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}


- (void)startTransaction:(SKPaymentTransaction *)transaction
{
    //DLog(@"starting transaction: %@", transaction);
}

- (void)failedTransaction: (SKPaymentTransaction *)transaction
{
    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
}

- (void)completeTransaction: (SKPaymentTransaction *)transaction
{
    if(IosBillingState.callbackFunction != nullptr)
    {
        std::string TransactionID = std::string([transaction.transactionIdentifier UTF8String]);
        std::string ProdID = std::string([transaction.payment.productIdentifier UTF8String]);

        IosBillingState.callbackFunction(TransactionID.c_str(), ProdID.c_str(), BillingManager::OperAction::Bought);

        [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
    }
    else
    {
        logError("callbackFunction for transaction update is not set");
    }
}

- (void)restoreTransaction: (SKPaymentTransaction *)transaction
{
    //[self recordTransaction:transaction];
    //[self purchaseSuccess:transaction.originalTransaction.payment.productIdentifier];

    if(IosBillingState.callbackObserver)
    {
        std::string TransactionID = std::string([transaction.originalTransaction.transactionIdentifier UTF8String]);
        std::string ProdID = std::string([transaction.originalTransaction.payment.productIdentifier UTF8String]);

        IosBillingState.callbackObserver->PurchaseUpdatedCallback(TransactionID.c_str(), ProdID.c_str(), BillingManager::OperAction::Restored);

        [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
    }
    else
    {
        logError("Callback for transaction update is not set");
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

bool iOSBillingManager::Init(BillingCallbackObserver* Observer)
{
    IosBillingState.callbackObserver = Observer;
   // _IosBillingProdRequestDelegate = [[IosBillingProdRequestDelegate alloc] init];
    [[FSProductStore defaultStore] registerObserver];

    return true;
};

bool iOSBillingManager::RestorePurchases()
{
    [[FSProductStore defaultStore] restoreTransactions];
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
};