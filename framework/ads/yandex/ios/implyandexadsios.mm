#import <YandexMobileAds/YandexMobileAds.h>
#include "implyandexadsios.h"
#include "basewrapper.h"

std::weak_ptr<ads::Yandex::iOSProvider> ads::Yandex::iOSProvider::staticProvider;
bool                                    ads::Yandex::iOSProvider::wasInited = false;

/*
    INTERSTITIAL CALLBACKS
*/
@interface iYandexInterstitial : NSObject
+(iYandexInterstitial*) defaultInterstitial;
@end

@interface iYandexInterstitial() <YMAInterstitialAdDelegate>
@property(nonatomic, strong) YMAInterstitialAd *interstitial;
@property(nonatomic)         std::string        UnitID;
-(void)SetUintId:(const char* )ID;
-(void)Load;
-(void)Show;
@end

@implementation iYandexInterstitial

+ (iYandexInterstitial *) defaultInterstitial
{
    static iYandexInterstitial* yandex;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (!yandex) {
            yandex = [[iYandexInterstitial alloc] init];
        }
    });
    return yandex;
}

-(void)SetUintId:(const char* )ID {
    self.UnitID = ID;
};

- (void)Load {
    if(!ads::Yandex::iOSProvider::wasInited) {
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
        return;
    }

    @autoreleasepool {
        try {
            NSString *ID = [[NSString alloc] initWithUTF8String:self.UnitID.c_str()];
            // self.interstitial = [[YMAInterstitialAd alloc] initWithAdUnitID:ID]; new version
            self.interstitial = [[YMAInterstitialAd alloc] initWithBlockID:ID]; // old version
            if(self.interstitial) {
                self.interstitial.delegate = self;
                [self.interstitial load];
            }
            else {
                ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
            }
        }
        catch (NSException * ex) {
            ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
        }
    }
}

-(void)Show {
    if (ads::Yandex::iOSProvider::wasInited && self.interstitial) {
        UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
        [self.interstitial presentFromViewController:controller];
        //[self.interstitial presentFromRootViewController:controller];
        return;
    }
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToShow);
};

- (void)interstitialAdDidLoad:(nonnull YMAInterstitialAd *)interstitialAd {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialLoaded);
}

- (void)interstitialAdDidFailToLoad:(nonnull YMAInterstitialAd *)interstitialAd error:(nonnull NSError *)error {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
}

- (void)interstitialAdDidFailToPresent:(nonnull YMAInterstitialAd *)interstitialAd error:(nonnull NSError *)error {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToShow);
}

- (void)interstitialAdDidAppear:(nonnull YMAInterstitialAd *)interstitialAd {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialOpened);
}

- (void)interstitialAdDidDisappear:(nonnull YMAInterstitialAd *)interstitialAd {
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialClosed);
}

- (void)interstitialAdWillLeaveApplication:(nonnull YMAInterstitialAd *)interstitialAd {}
- (void)interstitialAdWillAppear:(nonnull YMAInterstitialAd *)interstitialAd {}
- (void)interstitialAdWillDisappear:(nonnull YMAInterstitialAd *)interstitialAd {}
- (void)interstitialAdDidClick:(nonnull YMAInterstitialAd *)interstitialAd {}
- (void)interstitialAd:(nonnull YMAInterstitialAd *)interstitialAd willPresentScreen:(nullable UIViewController *)webBrowser {}
- (void)interstitialAd:(nonnull YMAInterstitialAd *)interstitialAd didTrackImpressionWithData:(nullable id<YMAImpressionData>)impressionData {}
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
    wasInited = true;
    interstitialStatus = ads::Provider::InterstitialStatus::ReadyToLoad;
    rewardedVideoStatus = ads::Provider::RewardedVideoStatus::ReadyToLoad;
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