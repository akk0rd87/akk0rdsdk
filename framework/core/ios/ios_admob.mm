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
@end

@interface iAdMob() <GADInterstitialDelegate>
-(void)InterstitialDestroy;
-(void)SendCallback:(const AdMob::AdEvent*) Event;
@property (atomic, strong) GADInterstitial          *interstitial;
@property (nonatomic)      AdMob::AdEventCallback   *Callback;
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
    // Do nothing
    logDebug("interstitialDidDismissScreen");
};

- (void)interstitialWillLeaveApplication:(nonnull GADInterstitial *)ad
{
    logDebug("interstitialWillLeaveApplication");
    AdMob::AdEvent Ad;
    Ad.AdFormat = (int)AdMob::Format::Interstitial;
    Ad.EventType = (int)AdMob::InterstitialEvent::LeftApplication;
    [self SendCallback: &Ad];
};

// interface API

-(void)Init
{
    self.interstitial = nullptr;
    self.Callback = nullptr;
};

-(void)InterstitialDestroy
{
    if(self.interstitial != nullptr)
    {
        [self.interstitial release];
        self.interstitial = nullptr;
    }
};

-(void)SetEventCallback:(AdMob::AdEventCallback*) EventCallback
{
    self.Callback = EventCallback;
};

-(void)InterstitialSetUnitID:(const char*) UnitID
{
    logDebug((std::string("InterstitialSetUnitID: ") + UnitID).c_str());
    [self InterstitialDestroy];
    NSString *ID = [[NSString alloc] initWithUTF8String:UnitID];
    self.interstitial = [[GADInterstitial alloc] initWithAdUnitID:ID];
    //self.interstitial = [[GADInterstitial alloc] initWithAdUnitID:@"ca-app-pub-3940256099942544/4411468910"];
    self.interstitial.delegate = self;
    [ID release];
};

-(void)InterstitialLoad
{
    logDebug("InterstitialLoad 1");
    if(self.interstitial != nullptr)
    {
       logDebug("InterstitialLoad 2");
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
