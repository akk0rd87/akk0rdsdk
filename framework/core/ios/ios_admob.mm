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
-(AdMob::InterstitialStatus) InterstitialGetStatus;
@end

@interface iAdMob() <GADInterstitialDelegate>
-(void)InterstitialDestroy;
@property (atomic, strong) GADInterstitial          *interstitial;
@property (atomic)         AdMob::InterstitialStatus interstitialStatus;
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

- (void)interstitialDidReceiveAd:(nonnull GADInterstitial *)ad
{
    self.interstitialStatus = AdMob::InterstitialStatus::Loaded;
};

- (void)interstitial:(nonnull GADInterstitial *)ad didFailToReceiveAdWithError:(nonnull GADRequestError *)error
{
    self.interstitialStatus = AdMob::InterstitialStatus::Inited;
};

- (void)interstitialWillPresentScreen:(nonnull GADInterstitial *)ad
{
    // Do nothing
};

- (void)interstitialDidFailToPresentScreen:(nonnull GADInterstitial *)ad
{
    self.interstitialStatus = AdMob::InterstitialStatus::Inited;
};

- (void)interstitialWillDismissScreen:(nonnull GADInterstitial *)ad
{
    // Do nothing
};

- (void)interstitialDidDismissScreen:(nonnull GADInterstitial *)ad
{
    self.interstitialStatus = AdMob::InterstitialStatus::Inited;
};

- (void)interstitialWillLeaveApplication:(nonnull GADInterstitial *)ad
{
    self.interstitialStatus = AdMob::InterstitialStatus::Inited;
};

// interface API

-(void)Init
{
    self.interstitial = nullptr;
    self.interstitialStatus = AdMob::InterstitialStatus::NotInited;
};

-(void)InterstitialDestroy
{
    if(self.interstitial != nullptr)
    {
        [self.interstitial release];
        self.interstitial = nullptr;
    }
};

-(void)InterstitialSetUnitID:(const char*) UnitID
{
    [self InterstitialDestroy];
    NSString *ID = [[NSString alloc] initWithUTF8String:UnitID];
    self.interstitial = [[GADInterstitial alloc] initWithAdUnitID:ID];
    self.interstitialStatus = AdMob::InterstitialStatus::Inited;
    [ID release];
};

-(void)InterstitialLoad
{
    if(self.interstitial != nullptr)
    {
       if(self.interstitialStatus == AdMob::InterstitialStatus::Inited)
       {
           self.interstitialStatus = AdMob::InterstitialStatus::TryingToLoad;
           GADRequest *request = [GADRequest request];
           [self.interstitial loadRequest:request];
       }
       else
       {
           logWarning("Wrong status to load interstitial ad");
       }
    }
    else
    {
        logError("Interstitial is empty");
    }
};

-(void)InterstitialShow
{
    if(self.interstitial != nullptr)
    {
        if([self.interstitial isReady])
        {
            self.interstitialStatus = AdMob::InterstitialStatus::TryingToShow;
           // [self.interstitial presentFromRootViewController:self];
            [self.interstitial presentFromRootViewController:nullptr];
        }
    }
    else
    {
        logError("Interstitial is empty");
    }
};

-(AdMob::InterstitialStatus) InterstitialGetStatus
{
    return AdMob::InterstitialStatus::NotInited;
};

@end


bool AdMobiOS::Init(const char* AdMobAppID, int Formats)
{
    NSString *AppID = [[NSString alloc] initWithUTF8String:AdMobAppID];
    [GADMobileAds configureWithApplicationID:AppID];
    [AppID release];
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

AdMob::InterstitialStatus  AdMobiOS::InterstitialGetStatus()
{
    return [[iAdMob defaultAdMob] InterstitialGetStatus];
};
