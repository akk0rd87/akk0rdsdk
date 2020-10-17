//#include "SDL.h"
#include "ios_admob.h"
#include "admob.h"
#import <GoogleMobileAds/GoogleMobileAds.h>

@interface iAdMob : NSObject
+ (iAdMob*) defaultAdMob;
-(void)Init;
-(void)InterstitialSetUnitID:(const char*) UnitID;
-(void)InterstitialLoad;
-(void)InterstitialShow;
-(void)SetEventCallback:(AdMob::AdEventCallback*) EventCallback;

-(void)RewardedVideoSetUnitID:(const char*) UnitID;
-(void)RewardedVideoLoad;
-(void)RewardedVideoShow;
@end

@interface iAdMob() <GADInterstitialDelegate, GADRewardedAdDelegate>
-(void)InterstitialDestroy;
-(void)InterstitialReset;
-(void)SendCallback:(const    AdMob::AdEvent*) Event;
@property (atomic, strong)    GADInterstitial          *interstitial;
@property (atomic, strong)    GADRewardedAd            *rewardedAd;
@property (nonatomic)         AdMob::AdEventCallback   *Callback;
@property (nonatomic)         std::string               InterstitialUnitID;
@property (nonatomic)         std::string               RewardedVideoUnitID;
@end

@implementation iAdMob
+ (iAdMob *) defaultAdMob
{
    static iAdMob* adMob;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (!adMob)
        {
            adMob = [[iAdMob alloc] init];
        }
    });
    return adMob;
}

-(void)SendCallback:(const AdMob::AdEvent*) Event
{
    if(self.Callback != nullptr)
    {
        self.Callback(Event);
    }
    else
    {
        logError("Ad Event Callback is empty");
    }
};


////
//// INTERSTITIAL DELEGATE CALLBACKS
////

- (void)interstitialDidReceiveAd:(nonnull GADInterstitial *)ad
{
    logDebug("interstitialDidReceiveAd");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::Interstitial;
    Ad.EventType = (int)AdMob::InterstitialEvent::Loaded;
    [self SendCallback: &Ad];
};

- (void)interstitial:(nonnull GADInterstitial *)ad didFailToReceiveAdWithError:(nonnull GADRequestError *)error
{
    logDebug("didFailToReceiveAdWithError");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::Interstitial;
    Ad.EventType = (int)AdMob::InterstitialEvent::Failed;
    [self SendCallback: &Ad];
};

- (void)interstitialWillPresentScreen:(nonnull GADInterstitial *)ad
{
    // Do nothing
    logDebug("interstitialWillPresentScreen");
};

- (void)interstitialDidFailToPresentScreen:(nonnull GADInterstitial *)ad
{
    logDebug("interstitialDidFailToPresentScreen");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::Interstitial;
    Ad.EventType = (int)AdMob::InterstitialEvent::Failed;
    [self SendCallback: &Ad];
};

- (void)interstitialWillDismissScreen:(nonnull GADInterstitial *)ad
{
    // Do nothing
    logDebug("interstitialWillDismissScreen");
};

- (void)interstitialDidDismissScreen:(nonnull GADInterstitial *)ad
{
    logDebug("interstitialDidDismissScreen");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::Interstitial;
    Ad.EventType = (int)AdMob::InterstitialEvent::Closed;
    [self SendCallback: &Ad];
};

- (void)interstitialWillLeaveApplication:(nonnull GADInterstitial *)ad
{
    logDebug("interstitialWillLeaveApplication");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::Interstitial;
    Ad.EventType = (int)AdMob::InterstitialEvent::LeftApplication;
    [self SendCallback: &Ad];
};

///////
///////  REWARDED VIDEO DELEGATE CALLBACKS
///////

- (void)rewardedAd:(GADRewardedAd *)rewardedAd userDidEarnReward:(GADAdReward *)reward {
    logDebug("Rewarded");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
    Ad.EventType = (int)AdMob::RewardedVideoEvent::Rewarded;
    [self SendCallback: &Ad];
}

- (void)rewardedAdDidPresent:(GADRewardedAd *)rewardedAd {
    logDebug("Opened reward based video ad.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
    Ad.EventType = (int)AdMob::RewardedVideoEvent::Opened;
    [self SendCallback: &Ad];
}


- (void)rewardedAdDidDismiss:(GADRewardedAd *)rewardedAd {
    logDebug("Reward based video ad is dismissed.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
    Ad.EventType = (int)AdMob::RewardedVideoEvent::Closed;
    [self SendCallback: &Ad];
}

- (void)rewardedAd:(GADRewardedAd *)rewardedAd didFailToPresentWithError:(NSError *)error {
    logDebug("Reward ad didFailToPresentWithError");
//    AdMob::AdEvent Ad;
//    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
//    Ad.EventType = (int)AdMob::RewardedVideoEvent::LeftApplication;
//    [self SendCallback: &Ad];
}

// interface API

-(void)Init
{
    [[GADMobileAds sharedInstance] startWithCompletionHandler:nil];
    self.interstitial = nullptr;
    self.rewardedAd = nullptr;
    self.Callback = nullptr;
//    [GADRewardBasedVideoAd sharedInstance].delegate = self;
};

-(void)InterstitialDestroy
{
    if(self.interstitial != nullptr)
    {
        [self.interstitial release];
        self.interstitial = nullptr;
    }
};

-(void)InterstitialReset
{
    [self InterstitialDestroy];
    NSString *ID = [[NSString alloc] initWithUTF8String:self.InterstitialUnitID.c_str()];
    self.interstitial = [[GADInterstitial alloc] initWithAdUnitID:ID];
    self.interstitial.delegate = self;
    [ID release];
}

-(void)SetEventCallback:(AdMob::AdEventCallback*) EventCallback
{
    self.Callback = EventCallback;
};

-(void)InterstitialSetUnitID:(const char*) UnitID
{
    logDebug((std::string("InterstitialSetUnitID: ") + UnitID).c_str());
    self.InterstitialUnitID = std::string(UnitID);
    [self InterstitialReset];
};

-(void)InterstitialLoad
{
    logDebug("InterstitialLoad 1");
    if(self.interstitial != nullptr)
    {
       logDebug("InterstitialLoad 2");

       if(self.interstitial.hasBeenUsed)
       {
           [self InterstitialReset];
       }

       GADRequest *request = [GADRequest request];
       //request.testDevices = @[ kGADSimulatorID ];
       [self.interstitial loadRequest:request];
    }
    else
    {
        logError("Interstitial is empty");
    }
};

-(void)InterstitialShow
{
    logDebug("InterstitialShow 1");
    if(self.interstitial != nullptr)
    {
        logDebug("InterstitialShow 2");
        if([self.interstitial isReady])
        {
            logDebug("InterstitialShow 3");
            //https://stackoverflow.com/questions/12418177/how-to-get-root-view-controller
            UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
            [self.interstitial presentFromRootViewController:controller];
        }
        else
        {
            logWarning("Interstitial is not ready for present");
        }
    }
    else
    {
        logError("Interstitial is empty");
    }
};

-(void)RewardedVideoSetUnitID:(const char*) UnitID
{
    self.RewardedVideoUnitID = std::string(UnitID);
};

-(void)RewardedVideoLoad
{
//    NSString *ID = [[NSString alloc] initWithUTF8String:self.RewardedVideoUnitID.c_str()];
//    [[GADRewardBasedVideoAd sharedInstance] loadRequest:[GADRequest request] withAdUnitID:ID];
//    [ID release];
    if(self.rewardedAd) {
        [self.rewardedAd release];
        self.rewardedAd = nullptr;
    };

    NSString *ID = [[NSString alloc] initWithUTF8String:self.RewardedVideoUnitID.c_str()];
    self.rewardedAd = [[GADRewardedAd alloc] initWithAdUnitID:ID];

    if(self.rewardedAd) {
        GADRequest *request = [GADRequest request];
        [self.rewardedAd loadRequest:request completionHandler:^(GADRequestError * _Nullable error) {
          AdMob::AdEvent Ad;
          Ad.AdFormat = static_cast<int>(AdMob::Format::RewardedVideo);
          if (error) {
            // Handle ad failed to load case.
              logDebug("Rewarded vided load failed");
              Ad.EventType = static_cast<int>(AdMob::RewardedVideoEvent::Failed);
          } else {
            // Ad successfully loaded.
              logDebug("Rewarded vided ad successfully loaded");
              Ad.EventType = static_cast<int>(AdMob::RewardedVideoEvent::Loaded);
          };
          [self SendCallback: &Ad];
        }];
    }
};

-(void)RewardedVideoShow
{
    /*
    if ([[GADRewardBasedVideoAd sharedInstance] isReady])
    {
        //https://stackoverflow.com/questions/12418177/how-to-get-root-view-controller
        UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
        [[GADRewardBasedVideoAd sharedInstance] presentFromRootViewController:controller];
    }
     */
    if(self.rewardedAd) {
        if(self.rewardedAd.isReady) {
            UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
            [self.rewardedAd presentFromRootViewController:controller delegate:self];
        }
    }
};

@end


bool AdMobiOS::Init(const char* AdMobAppID, int Formats)
{
    [[iAdMob defaultAdMob] Init];
    return true;
}

bool AdMobiOS::InterstitialSetUnitId(const char* UnitId)
{
    [[iAdMob defaultAdMob] InterstitialSetUnitID:UnitId];
    return true;
};

bool AdMobiOS::InterstitialLoad()
{
    [[iAdMob defaultAdMob] InterstitialLoad];
    return true;
};

bool AdMobiOS::InterstitialShow()
{
    [[iAdMob defaultAdMob] InterstitialShow];
    return true;
};

void AdMobiOS::SetAdEventCallback (AdMob::AdEventCallback * Callback)
{
    [[iAdMob defaultAdMob] SetEventCallback: Callback];
};

bool AdMobiOS::RewardedVideoSetUnitId(const char* UnitId)
{
    [[iAdMob defaultAdMob] RewardedVideoSetUnitID:UnitId];
    return true;
};

bool AdMobiOS::RewardedVideoLoad()
{
    [[iAdMob defaultAdMob] RewardedVideoLoad];
    return true;
};

bool AdMobiOS::RewardedVideoShow()
{
    [[iAdMob defaultAdMob] RewardedVideoShow];
    return true;
};
