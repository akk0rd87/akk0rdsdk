#import <GoogleMobileAds/GoogleMobileAds.h>
#include "impladmobios.h"
#include "basewrapper.h"
#include "core/platformwrapper/ios/ios_wrapper.h"

std::weak_ptr<ads::AdMob::iOSProvider> ads::AdMob::iOSProvider::staticProvider;
bool                                   ads::AdMob::iOSProvider::wasInited = false;

/*
    INTERSTITIAL CALLBACKS
*/
@interface iAdMobInterstitial : NSObject
+(iAdMobInterstitial*) defaultInterstitial;
@end

@interface iAdMobInterstitial() <GADFullScreenContentDelegate>
@property(nonatomic, strong) GADInterstitialAd *interstitial;
@property(nonatomic)         std::string        UnitID;
-(void)SetUintId:(const char* )ID;
-(void)Load;
-(void)Show;
@end

@implementation iAdMobInterstitial

+ (iAdMobInterstitial *) defaultInterstitial
{
    static iAdMobInterstitial* adMob;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (!adMob) {
            adMob = [[iAdMobInterstitial alloc] init];
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
        auto controller = iosWrapper::getRootViewController();
        if(controller) {
            if ([self.interstitial canPresentFromRootViewController :controller error:nil]) {
                [self.interstitial presentFromRootViewController:controller];
                return;
            }
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
@interface iAdMobRewardedVideo : NSObject
+(iAdMobRewardedVideo*) defaultRewardedVideo;
@end

@interface iAdMobRewardedVideo() <GADFullScreenContentDelegate>
@property(nonatomic, strong) GADRewardedAd *rewardedAd;
@property(nonatomic)         std::string        UnitID;
-(void)SetUintId:(const char* )ID;
-(void)Load;
-(void)Show;
@end

@implementation iAdMobRewardedVideo

+ (iAdMobRewardedVideo *) defaultRewardedVideo
{
    static iAdMobRewardedVideo* adMob;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (!adMob) {
            adMob = [[iAdMobRewardedVideo alloc] init];
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
        auto controller = iosWrapper::getRootViewController();
        if(controller) {
            if([self.rewardedAd canPresentFromRootViewController:controller error:nil]) {
                [self.rewardedAd presentFromRootViewController:controller
                                    userDidEarnRewardHandler:^ {
                    logDebug("Rewarded");
                    ads::AdMob::iOSProvider::onAdEvent(ads::Event::RewardedVideoRewarded);
                }];
                return;
            }
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
        interstitialStatus = ads::InterstitialStatus::ReadyToLoad;
        rewardedVideoStatus = ads::RewardedVideoStatus::ReadyToLoad;
    }];
};

void ads::AdMob::iOSProvider::InterstitialSetUnitId(const std::string& unitId) {
    [[iAdMobInterstitial defaultInterstitial] SetUintId: unitId.c_str()];
}

void ads::AdMob::iOSProvider::setRewardedVideoUnit(const char* unitId) {
    [[iAdMobRewardedVideo defaultRewardedVideo] SetUintId: unitId];
}

void ads::AdMob::iOSProvider::v_tryLoadInterstitial() {
    [[iAdMobInterstitial defaultInterstitial] Load];
}

void ads::AdMob::iOSProvider::v_tryLoadRewardedVideo() {
    [[iAdMobRewardedVideo defaultRewardedVideo] Load];
}

void ads::AdMob::iOSProvider::v_showInterstitial() {
    [[iAdMobInterstitial defaultInterstitial] Show];
}

void ads::AdMob::iOSProvider::v_showRewardedVideo() {
    [[iAdMobRewardedVideo defaultRewardedVideo] Show];
}
