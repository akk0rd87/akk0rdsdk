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

@interface iAdMob() <GADInterstitialDelegate, GADRewardBasedVideoAdDelegate>
-(void)InterstitialDestroy;
-(void)InterstitialReset;
-(void)SendCallback:(const    AdMob::AdEvent*) Event;
@property (atomic, strong)    GADInterstitial          *interstitial;
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

-(void)rewardBasedVideoAd:(GADRewardBasedVideoAd *)rewardBasedVideoAd didRewardUserWithReward:(GADAdReward *)reward {
    logDebug("Rewarded");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
    Ad.EventType = (int)AdMob::RewardedVideoEvent::Rewarded;
    [self SendCallback: &Ad];
}

- (void)rewardBasedVideoAdDidReceiveAd:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    logDebug("Reward based video ad is received.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
    Ad.EventType = (int)AdMob::RewardedVideoEvent::Loaded;
    [self SendCallback: &Ad];
}

- (void)rewardBasedVideoAdDidOpen:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    logDebug("Opened reward based video ad.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
    Ad.EventType = (int)AdMob::RewardedVideoEvent::Opened;
    [self SendCallback: &Ad];
}

- (void)rewardBasedVideoAdDidStartPlaying:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    logDebug("Reward based video ad started playing.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
    Ad.EventType = (int)AdMob::RewardedVideoEvent::Started;
    [self SendCallback: &Ad];
}

- (void)rewardBasedVideoAdDidCompletePlaying:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    logDebug("Reward based video ad has completed.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
    Ad.EventType = (int)AdMob::RewardedVideoEvent::Completed;
    [self SendCallback: &Ad];
}

- (void)rewardBasedVideoAdDidClose:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    logDebug("Reward based video ad is closed.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
    Ad.EventType = (int)AdMob::RewardedVideoEvent::Closed;
    [self SendCallback: &Ad];
}

- (void)rewardBasedVideoAdWillLeaveApplication:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    logDebug("Reward based video ad will leave application.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
    Ad.EventType = (int)AdMob::RewardedVideoEvent::LeftApplication;
    [self SendCallback: &Ad];
}

- (void)rewardBasedVideoAd:(GADRewardBasedVideoAd *)rewardBasedVideoAd didFailToLoadWithError:(NSError *)error {
    logDebug("Reward based video ad failed to load.");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::RewardedVideo;
    Ad.EventType = (int)AdMob::RewardedVideoEvent::Failed;
    [self SendCallback: &Ad];
}

// interface API

-(void)Init
{
    self.interstitial = nullptr;
    self.Callback = nullptr;
    [GADRewardBasedVideoAd sharedInstance].delegate = self;
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
    NSString *ID = [[NSString alloc] initWithUTF8String:self.RewardedVideoUnitID.c_str()];
    [[GADRewardBasedVideoAd sharedInstance] loadRequest:[GADRequest request] withAdUnitID:ID];
    [ID release];
};

-(void)RewardedVideoShow
{
    if ([[GADRewardBasedVideoAd sharedInstance] isReady])
    {
        //https://stackoverflow.com/questions/12418177/how-to-get-root-view-controller
        UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
        [[GADRewardBasedVideoAd sharedInstance] presentFromRootViewController:controller];
    }
};

@end


bool AdMobiOS::Init(const char* AdMobAppID, int Formats)
{
    NSString *AppID = [[NSString alloc] initWithUTF8String:AdMobAppID];
    [GADMobileAds configureWithApplicationID:AppID];
    //[[iAdMob defaultAdMob] SetEventCallback: Callback];
    [AppID release];
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
