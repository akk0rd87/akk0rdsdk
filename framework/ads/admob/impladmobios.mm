#import <GoogleMobileAds/GoogleMobileAds.h>
#include "impladmobios.h"
#include "basewrapper.h"

std::weak_ptr<ads::AdMob::iOSProvider> ads::AdMob::iOSProvider::staticProvider;
bool                                   ads::AdMob::iOSProvider::wasInited = false;

/*
    INTERSTITIAL CALLBACKS
*/
@interface iInterstitial : NSObject
+(iInterstitial*) defaultInterstitial;
@end

@interface iInterstitial() <GADFullScreenContentDelegate>
@property(nonatomic, strong) GADInterstitialAd *interstitial;
@property(nonatomic)         std::string        UnitID;
-(void)SetUintId:(const char* )ID;
-(void)Load;
-(void)Show;
@end

@implementation iInterstitial

+ (iInterstitial *) defaultInterstitial
{
    static iInterstitial* adMob;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (!adMob) {
            adMob = [[iInterstitial alloc] init];
        }
    });
    return adMob;
}

-(void)SetUintId:(const char* )ID {
    self.UnitID = ID;
};

- (void)Load {
    if(!ads::AdMob::iOSProvider::wasInited) {
        ads::AdMob::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
        return;
    }

    GADRequest *request = [GADRequest request];
    if(!request) {
        ads::AdMob::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
        return;
    }

    @autoreleasepool {
        NSString *ID = [[NSString alloc] initWithUTF8String:self.UnitID.c_str()];
        [GADInterstitialAd loadWithAdUnitID:ID
                                    request:request
                            completionHandler:^(GADInterstitialAd *ad, NSError *error) {
            if (error) {
                logDebug("InterstitialLoad load error");
                if (!BWrapper::IsReleaseBuild()) {
                    NSLog(@"Failed to load interstitial ad with error: %@", [error localizedDescription]);
                }
                ads::AdMob::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
            }
            else {
                logDebug("InterstitialLoad load success");
                self.interstitial = ad;
                self.interstitial.fullScreenContentDelegate = self;
                ads::AdMob::iOSProvider::onAdEvent(ads::Event::InterstitialLoaded);
            }
        }];
    }
}

-(void)Show {
    if (ads::AdMob::iOSProvider::wasInited && self.interstitial) {
        UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
        if ([self.interstitial canPresentFromRootViewController :controller error:nil]) {
            [self.interstitial presentFromRootViewController:controller];
            return;
        }
    }
    ads::AdMob::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToShow);
};

/// Tells the delegate that the ad failed to present full screen content.
- (void)ad:(nonnull id<GADFullScreenPresentingAd>)ad
didFailToPresentFullScreenContentWithError:(nonnull NSError *)error {
    logDebug("Ad did fail to present full screen content.");
    ads::AdMob::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToShow);
}

/// Tells the delegate that the ad presented full screen content.
- (void)adDidPresentFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    logDebug("Ad did present full screen content.");
}

/// Tells the delegate that the ad dismissed full screen content.
- (void)adDidDismissFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    logDebug("Ad did dismiss full screen content.");
    ads::AdMob::iOSProvider::onAdEvent(ads::Event::InterstitialClosed);
}

- (void)adDidRecordImpression:(nonnull id<GADFullScreenPresentingAd>)ad {
    logDebug("adDidRecordImpression");
}
@end

/*
    REWARDED CALLBACKS
*/
@interface iRewardedVideo : NSObject
+(iRewardedVideo*) defaultRewardedVideo;
@end

@interface iRewardedVideo() <GADFullScreenContentDelegate>
@property(nonatomic, strong) GADRewardedAd *rewardedAd;
@property(nonatomic)         std::string        UnitID;
-(void)SetUintId:(const char* )ID;
-(void)Load;
-(void)Show;
@end

@implementation iRewardedVideo

+ (iRewardedVideo *) defaultRewardedVideo
{
    static iRewardedVideo* adMob;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (!adMob) {
            adMob = [[iRewardedVideo alloc] init];
        }
    });
    return adMob;
}

-(void)SetUintId:(const char* )ID {
    self.UnitID = ID;
};

- (void)Load {
    if(!ads::AdMob::iOSProvider::wasInited) {
        ads::AdMob::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToLoad);
        return;
    }

    GADRequest *request = [GADRequest request];
    if(!request) {
        ads::AdMob::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToLoad);
        return;
    }

    @autoreleasepool {
        NSString *ID = [[NSString alloc] initWithUTF8String:self.UnitID.c_str()];
        [GADRewardedAd
                loadWithAdUnitID:ID
                        request:request
            completionHandler:^(GADRewardedAd *ad, NSError *error) {
                if (error) {
                    logDebug("Rewarded failed load");
                    ads::AdMob::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToLoad);
                }
                else {
                    logDebug("Rewarded success load");
                    self.rewardedAd = ad;
                    self.rewardedAd.fullScreenContentDelegate = self;
                    ads::AdMob::iOSProvider::onAdEvent(ads::Event::RewardedVideoLoaded);
                }
            }];
    }
}

-(void)Show {
    if(ads::AdMob::iOSProvider::wasInited && self.rewardedAd) {
        UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
        if([self.rewardedAd canPresentFromRootViewController:controller error:nil]) {
            [self.rewardedAd presentFromRootViewController:controller
                                  userDidEarnRewardHandler:^ {
                logDebug("Rewarded");
                ads::AdMob::iOSProvider::onAdEvent(ads::Event::RewardedVideoRewarded);
            }];
            return;
        }
    }
    ads::AdMob::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToShow);
}

/// Tells the delegate that the ad failed to present full screen content.
- (void)ad:(nonnull id<GADFullScreenPresentingAd>)ad
didFailToPresentFullScreenContentWithError:(nonnull NSError *)error {
    logDebug("Ad did fail to present full screen content.");
    ads::AdMob::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToShow);
}

/// Tells the delegate that the ad presented full screen content.
- (void)adDidPresentFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    logDebug("Ad did present full screen content.");
}

/// Tells the delegate that the ad dismissed full screen content.
- (void)adDidDismissFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    logDebug("Ad did dismiss full screen content.");
    ads::AdMob::iOSProvider::onAdEvent(ads::Event::RewardedVideoClosed);
}

- (void)adDidRecordImpression:(nonnull id<GADFullScreenPresentingAd>)ad {
    logDebug("adDidRecordImpression");
}
@end

ads::AdMob::iOSProvider::iOSProvider(std::weak_ptr<ads::ProviderCallback> cbk, ads::Format format) :
ads::AdMob::Provider (cbk) {
    wasInited = false;
    [[GADMobileAds sharedInstance] startWithCompletionHandler:^(GADInitializationStatus *_Nonnull) {
        wasInited = true;
        interstitialStatus = ads::Provider::InterstitialStatus::ReadyToLoad;
        rewardedVideoStatus = ads::Provider::RewardedVideoStatus::ReadyToLoad;
    }];
};

void ads::AdMob::iOSProvider::InterstitialSetUnitId(const std::string& unitId) {
    [[iInterstitial defaultInterstitial] SetUintId: unitId.c_str()];
}

void ads::AdMob::iOSProvider::setRewardedVideoUnit(const char* unitId) {
    [[iRewardedVideo defaultRewardedVideo] SetUintId: unitId];
}

void ads::AdMob::iOSProvider::admob_tryLoadInterstitial() {
    [[iInterstitial defaultInterstitial] Load];
}

void ads::AdMob::iOSProvider::admob_tryLoadRewardedVideo() {
    [[iRewardedVideo defaultRewardedVideo] Load];
}

void ads::AdMob::iOSProvider::v_showInterstitial() {
    [[iInterstitial defaultInterstitial] Show];
}

void ads::AdMob::iOSProvider::v_showRewardedVideo() {
    [[iRewardedVideo defaultRewardedVideo] Show];
}
