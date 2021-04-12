#include "ios_admob.h"
#include "admob.h"
#import <GoogleMobileAds/GoogleMobileAds.h>

AdMob::AdEventCallback *admobCallback = nullptr;
static void sendCallback(const AdMob::AdEvent& CallbackEvent) {
    if(admobCallback) {
        admobCallback(&CallbackEvent);
    }
    else {
        logError("iOS Admob Callback is not set");
    }
};

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
    GADRequest *request = [GADRequest request];
    if(request) {
        @autoreleasepool {
            NSString *ID = [[NSString alloc] initWithUTF8String:self.UnitID.c_str()];
            [GADInterstitialAd loadWithAdUnitID:ID
                                        request:request
                              completionHandler:^(GADInterstitialAd *ad, NSError *error) {
                AdMob::AdEvent Ad;
                Ad.AdFormat = static_cast<decltype(Ad.AdFormat)>(AdMob::Format::Interstitial);
                using event = AdMob::InterstitialEvent;
              if (error) {
                  logDebug("InterstitialLoad success load");
                  //NSLog(@"Failed to load interstitial ad with error: %@", [error localizedDescription]);
                  Ad.EventType = static_cast<decltype(Ad.EventType)>(event::Failed);
              }
              else {
                  Ad.EventType = static_cast<decltype(Ad.EventType)>(event::Loaded);
                  logDebug("InterstitialLoad success load");
                  self.interstitial = ad;
                  self.interstitial.fullScreenContentDelegate = self;
              }
              sendCallback(Ad);
            }];
        }
    }
}

-(void)Show {
    if (self.interstitial) {
        UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
        if ([self.interstitial canPresentFromRootViewController :controller error:nil]) {
            [self.interstitial presentFromRootViewController:controller];
        }
    }
};

/// Tells the delegate that the ad failed to present full screen content.
- (void)ad:(nonnull id<GADFullScreenPresentingAd>)ad
didFailToPresentFullScreenContentWithError:(nonnull NSError *)error {
    logDebug("Ad did fail to present full screen content.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = static_cast<decltype(Ad.AdFormat)>(AdMob::Format::Interstitial);
    Ad.EventType = static_cast<decltype(Ad.EventType)>(AdMob::InterstitialEvent::Failed);
    sendCallback(Ad);
}

/// Tells the delegate that the ad presented full screen content.
- (void)adDidPresentFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    logDebug("Ad did present full screen content.");
}

/// Tells the delegate that the ad dismissed full screen content.
- (void)adDidDismissFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    logDebug("Ad did dismiss full screen content.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = static_cast<decltype(Ad.AdFormat)>(AdMob::Format::Interstitial);
    Ad.EventType = static_cast<decltype(Ad.EventType)>(AdMob::InterstitialEvent::Closed);
    sendCallback(Ad);
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
    GADRequest *request = [GADRequest request];
    if(request) {
        @autoreleasepool {
            NSString *ID = [[NSString alloc] initWithUTF8String:self.UnitID.c_str()];
            [GADRewardedAd
                 loadWithAdUnitID:ID
                          request:request
                completionHandler:^(GADRewardedAd *ad, NSError *error) {
                  AdMob::AdEvent Ad;
                  Ad.AdFormat = static_cast<decltype(Ad.AdFormat)>(AdMob::Format::RewardedVideo);
                  using event = AdMob::RewardedVideoEvent;
                  if (error) {
                      logDebug("Rewarded failed load");
                      //NSLog(@"Rewarded ad failed to load with error: %@", [error localizedDescription]);
                      Ad.EventType = static_cast<decltype(Ad.EventType)>(event::Failed);
                  }
                  else {
                      logDebug("Rewarded success load");
                      self.rewardedAd = ad;
                      self.rewardedAd.fullScreenContentDelegate = self;
                      Ad.EventType = static_cast<decltype(Ad.EventType)>(event::Loaded);
                  }
                  sendCallback(Ad);
                }];
        }
    }
}

-(void)Show {
    if(self.rewardedAd) {
        UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
        if([self.rewardedAd canPresentFromRootViewController:controller error:nil]) {
            [self.rewardedAd presentFromRootViewController:controller
                                  userDidEarnRewardHandler:^ {
                logDebug("Rewarded");
                AdMob::AdEvent Ad;
                Ad.AdFormat = static_cast<decltype(Ad.AdFormat)>(AdMob::Format::RewardedVideo);
                Ad.EventType = static_cast<decltype(Ad.EventType)>(AdMob::RewardedVideoEvent::Rewarded);
                sendCallback(Ad);
            }];
        }
    }
}

/// Tells the delegate that the ad failed to present full screen content.
- (void)ad:(nonnull id<GADFullScreenPresentingAd>)ad
didFailToPresentFullScreenContentWithError:(nonnull NSError *)error {
    logDebug("Ad did fail to present full screen content.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = static_cast<decltype(Ad.AdFormat)>(AdMob::Format::RewardedVideo);
    Ad.EventType = static_cast<decltype(Ad.EventType)>(AdMob::RewardedVideoEvent::Failed);
    sendCallback(Ad);
}

/// Tells the delegate that the ad presented full screen content.
- (void)adDidPresentFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    logDebug("Ad did present full screen content.");
}

/// Tells the delegate that the ad dismissed full screen content.
- (void)adDidDismissFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    logDebug("Ad did dismiss full screen content.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = static_cast<decltype(Ad.AdFormat)>(AdMob::Format::RewardedVideo);
    Ad.EventType = static_cast<decltype(Ad.EventType)>(AdMob::RewardedVideoEvent::Closed);
    sendCallback(Ad);
}

- (void)adDidRecordImpression:(nonnull id<GADFullScreenPresentingAd>)ad {
    logDebug("adDidRecordImpression");
}
@end

//
// INTERFACE METHODS
//
bool AdMobiOS::Init(AdMob::Format Formats) {
    [[GADMobileAds sharedInstance] startWithCompletionHandler:nil];
    return true;
}

bool AdMobiOS::InterstitialSetUnitId(const char* UnitId) {
    [[iInterstitial defaultInterstitial] SetUintId: UnitId];
    return true;
};

bool AdMobiOS::InterstitialLoad() {
    [[iInterstitial defaultInterstitial] Load];
    return true;
};

bool AdMobiOS::InterstitialShow() {
    [[iInterstitial defaultInterstitial] Show];
    return true;
};

void AdMobiOS::SetAdEventCallback (AdMob::AdEventCallback * Callback) {
    admobCallback = Callback;
};

bool AdMobiOS::RewardedVideoSetUnitId(const char* UnitId) {
    [[iRewardedVideo defaultRewardedVideo] SetUintId: UnitId];
    return true;
};

bool AdMobiOS::RewardedVideoLoad() {
    [[iRewardedVideo defaultRewardedVideo] Load];
    return true;
};

bool AdMobiOS::RewardedVideoShow() {
    [[iRewardedVideo defaultRewardedVideo] Show];
    return true;
};
