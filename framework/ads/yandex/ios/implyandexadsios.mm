#import <YandexMobileAds/YandexMobileAds.h>
#include "implyandexadsios.h"
#include "basewrapper.h"

/*
https://yandex.ru/support2/mobile-ads/ru/dev/ios
https://yandex.ru/support2/mobile-ads/ru/dev/ios/refs
https://yastatic.net/s3/doc-binary/src/dev/mobile-ads/ru/jazzy/index.html
https://yandex.ru/support2/mobile-ads/ru/dev/ios/quick-start
https://yandex.ru/support2/mobile-ads/ru/dev/ios/support-ios14
https://yandex.ru/support2/mobile-ads/ru/dev/ios/example-ios
https://github.com/yandexmobile/yandex-ads-sdk-ios/tree/master
*/

std::weak_ptr<ads::Yandex::iOSProvider> ads::Yandex::iOSProvider::staticProvider;
bool                                    ads::Yandex::iOSProvider::wasInited = false;

/*
    INTERSTITIAL CALLBACKS
*/
@interface iYandexInterstitial : NSObject
+(iYandexInterstitial*) defaultInterstitial;
@end

@interface iYandexInterstitial() <YMAInterstitialAdLoaderDelegate, YMAInterstitialAdDelegate>
@property(nonatomic, strong) YMAInterstitialAd *interstitial;
@property(nonatomic, strong) YMAInterstitialAdLoader *loader;
@property(nonatomic, strong) YMAAdRequestConfiguration* cfg;
@property(nonatomic)         std::string        UnitID;
-(void)Init;
-(void)SetUintId:(const char* )ID;
-(void)Load;
-(void)Show;
@end

@implementation iYandexInterstitial

+ (iYandexInterstitial *) defaultInterstitial
{
    static iYandexInterstitial* yandex;
    if (!yandex) {
        yandex = [[iYandexInterstitial alloc] init];
        [yandex Init];
    }
    return yandex;
}

-(void)Init {
    self.loader = [[YMAInterstitialAdLoader alloc] init];
    self.loader.delegate = self;
};

-(void)SetUintId:(const char* )IDstr {
    try {
        if(self.cfg) {
            [self.cfg release];
        }
        NSString *ID = [[NSString alloc] initWithUTF8String:IDstr];
        if(ID) {
            self.cfg = [[YMAAdRequestConfiguration alloc] initWithAdUnitID:ID];
            [ID release];
        }
    }
    catch (NSException * ex) {
        
    }
};

- (void)Load {
    if(!ads::Yandex::iOSProvider::wasInited || !self.cfg) {
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
        return;
    }

    try {
        [self.loader loadAdWithRequestConfiguration: self.cfg];
    }
    catch (NSException * ex) {
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
    }
}

-(void)Show {
    try {
        if (ads::Yandex::iOSProvider::wasInited && self.interstitial) {
            UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
            [self.interstitial showFromViewController:controller];
            return;
        }
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToShow);
    }
    catch (NSException * ex) {
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
    }
};

- (void)interstitialAdLoader:(YMAInterstitialAdLoader *)adLoader
                     didLoad:(YMAInterstitialAd *)interstitialAd {
    if(interstitialAd) {
        self.interstitial = interstitialAd;
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialLoaded);
        return;
    }
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
}

- (void)interstitialAdLoader:(YMAInterstitialAdLoader *)adLoader
      didFailToLoadWithError:(YMAAdRequestError *)error {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
}


/**
 Notifies that the ad can't be displayed.
 @param interstitialAd A reference to an object of the YMAInterstitialAd class that invoked the method.
 @param error Information about the error (for details, see YMAAdErrorCode).
 */
- (void)interstitialAd:(YMAInterstitialAd *)interstitialAd didFailToShowWithError:(NSError *)error {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToShow);
}

/**
 Called after the interstitial ad shows.
 @param interstitialAd A reference to an object of the YMAInterstitialAd class that invoked the method.
 */
- (void)interstitialAdDidShow:(YMAInterstitialAd *)interstitialAd {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialOpened);
}

/**
 Called after dismissing the interstitial ad.
 @param interstitialAd A reference to an object of the YMAInterstitialAd class that invoked the method.
 */
- (void)interstitialAdDidDismiss:(YMAInterstitialAd *)interstitialAd {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialClosed);
}

/**
 Notifies that the user has clicked on the ad.
 @param interstitialAd A reference to an object of the YMAInterstitialAd class that invoked the method.
 */
- (void)interstitialAdDidClick:(YMAInterstitialAd *)interstitialAd {

}

/**
 Notifies delegate when an impression was tracked.
 @param interstitialAd A reference to an object of the YMAInterstitialAd class that invoked the method.
 @param impressionData Ad impression-level revenue data.
 */
- (void)interstitialAd:(YMAInterstitialAd *)interstitialAd
didTrackImpressionWithData:(nullable id<YMAImpressionData>)impressionData {

}
@end

/*
    REWARDED CALLBACKS
*/
@interface iYandexRewardedVideo : NSObject
+(iYandexRewardedVideo*) defaultRewardedVideo;
@end

@interface iYandexRewardedVideo() <YMARewardedAdDelegate>
@property(nonatomic, strong) YMARewardedAd *rewardedAd;
@property(nonatomic)         std::string        UnitID;
-(void)SetUintId:(const char* )ID;
-(void)Load;
-(void)Show;
@end

@implementation iYandexRewardedVideo

+ (iYandexRewardedVideo *) defaultRewardedVideo
{
    static iYandexRewardedVideo* yandex;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (!yandex) {
            yandex = [[iYandexRewardedVideo alloc] init];
        }
    });
    return yandex;
}

-(void)SetUintId:(const char* )ID {
    self.UnitID = ID;
};

- (void)Load {
    if(!ads::Yandex::iOSProvider::wasInited) {
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToLoad);
        return;
    }

    @autoreleasepool {
        try {
            NSString *ID = [[NSString alloc] initWithUTF8String:self.UnitID.c_str()];
            //self.rewardedAd = [[YMARewardedAd alloc] initWithAdUnitID:ID]; new version
            self.rewardedAd = [[YMARewardedAd alloc] initWithBlockID:ID]; // old version
            if(self.rewardedAd) {
                self.rewardedAd.delegate = self;
                [self.rewardedAd load];
            }
            else {
                ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToLoad);
            }
        }
        catch (NSException * ex) {
            ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
        }
    }
}

-(void)Show {
    if(ads::Yandex::iOSProvider::wasInited && self.rewardedAd) {
        UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
        [self.rewardedAd presentFromViewController : controller];
        return;
    }
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToShow);
}

- (void)rewardedAd:(nonnull YMARewardedAd *)rewardedAd didReward:(nonnull id<YMAReward>)reward {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoRewarded);
}

- (void)rewardedAdDidLoad:(nonnull YMARewardedAd *)rewardedAd {
    logDebug("RewardedVideoLoaded");
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoLoaded);
}

- (void)rewardedAdDidFailToLoad:(nonnull YMARewardedAd *)rewardedAd error:(nonnull NSError *)error {
    logDebug("RewardedVideoFailedToLoad");
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToLoad);
}

- (void)rewardedAdDidFailToPresent:(nonnull YMARewardedAd *)rewardedAd error:(nonnull NSError *)error {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToShow);
}

- (void)rewardedAdDidDisappear:(nonnull YMARewardedAd *)rewardedAd {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoClosed);
}

- (void)rewardedAdDidAppear:(nonnull YMARewardedAd *)rewardedAd {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoOpened);
}

- (void)rewardedAdWillLeaveApplication:(nonnull YMARewardedAd *)rewardedAd {}
- (void)rewardedAdWillAppear:(nonnull YMARewardedAd *)rewardedAd {}
- (void)rewardedAdWillDisappear:(nonnull YMARewardedAd *)rewardedAd {}
- (void)rewardedAdDidClick:(nonnull YMARewardedAd *)rewardedAd {}
- (void)rewardedAd:(nonnull YMARewardedAd *)rewardedAd willPresentScreen:(nullable UIViewController *)viewController {}
- (void)rewardedAd:(nonnull YMARewardedAd *)rewardedAd didTrackImpressionWithData:(nullable id<YMAImpressionData>)impressionData {}
@end

ads::Yandex::iOSProvider::iOSProvider(std::weak_ptr<ads::ProviderCallback> cbk, ads::Format format) :
ads::Yandex::Provider (cbk) {
    [YMAMobileAds initializeSDKWithCompletionHandler:^{
        wasInited = true;
        interstitialStatus = ads::InterstitialStatus::ReadyToLoad;
        rewardedVideoStatus = ads::RewardedVideoStatus::ReadyToLoad;
    }];
};

void ads::Yandex::iOSProvider::InterstitialSetUnitId(const std::string& unitId) {
    [[iYandexInterstitial defaultInterstitial] SetUintId: unitId.c_str()];
}

void ads::Yandex::iOSProvider::setRewardedVideoUnit(const char* unitId) {
    [[iYandexRewardedVideo defaultRewardedVideo] SetUintId: unitId];
}

void ads::Yandex::iOSProvider::v_tryLoadInterstitial() {
    [[iYandexInterstitial defaultInterstitial] Load];
}

void ads::Yandex::iOSProvider::v_tryLoadRewardedVideo() {
    [[iYandexRewardedVideo defaultRewardedVideo] Load];
}

void ads::Yandex::iOSProvider::v_showInterstitial() {
    [[iYandexInterstitial defaultInterstitial] Show];
}

void ads::Yandex::iOSProvider::v_showRewardedVideo() {
    [[iYandexRewardedVideo defaultRewardedVideo] Show];
}
