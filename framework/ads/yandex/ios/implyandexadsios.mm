#import <YandexMobileAds/YandexMobileAds.h>
#import <YandexMobileAds/YandexMobileAds-Swift.h>
#include "implyandexadsios.h"
#include "basewrapper.h"
#include "core/platformwrapper/ios/ios_wrapper.h"

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
        logDebug("Create yandex interstitial class");
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
    logDebug("Interstitial SetUintId %s", IDstr);
    try {
        NSString *ID = [[NSString alloc] initWithUTF8String:IDstr];
        if(ID) {
            self.cfg = [[YMAAdRequestConfiguration alloc] initWithAdUnitID:ID];
        }
    }
    catch (NSException * ex) {
        logDebug("Interstitial SetUintId error");
    }
};

- (void)Load {
    logDebug("Interstitial Load");
    if(!ads::Yandex::iOSProvider::wasInited || !self.cfg || !self.loader) {
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
    logDebug("Interstitial Show");
    try {
        if (ads::Yandex::iOSProvider::wasInited && self.interstitial) {
            auto controller = iosWrapper::getRootViewController();
            if(controller) {
                self.interstitial.delegate = self;
                [self.interstitial showFromViewController:controller];
                return;
            }
        }
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToShow);
    }
    catch (NSException * ex) {
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
    }
};

- (void)interstitialAdLoader:(YMAInterstitialAdLoader *)adLoader didLoad:(YMAInterstitialAd *)interstitialAd {
    logDebug("interstitialAdLoader:didLoad");
    if(interstitialAd) {
        self.interstitial = interstitialAd;
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialLoaded);
        return;
    }
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
}

- (void)interstitialAdLoader:(YMAInterstitialAdLoader *)adLoader didFailToLoadWithError:(YMAAdRequestError *)error {
    logDebug("interstitialAdLoader:didFailToLoadWithError: %s", [[error.error localizedDescription] UTF8String]);
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToLoad);
}


/**
 Notifies that the ad can't be displayed.
 @param interstitialAd A reference to an object of the YMAInterstitialAd class that invoked the method.
 @param error Information about the error (for details, see YMAAdErrorCode).
 */
- (void)interstitialAd:(YMAInterstitialAd *)interstitialAd didFailToShowWithError:(NSError *)error {
    logDebug("didFailToShowWithError: %s", [[error localizedDescription] UTF8String]);
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialFailedToShow);
}

/**
 Called after the interstitial ad shows.
 @param interstitialAd A reference to an object of the YMAInterstitialAd class that invoked the method.
 */
- (void)interstitialAdDidShow:(YMAInterstitialAd *)interstitialAd {
    logDebug("interstitialAdDidShow");
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialOpened);
}

/**
 Called after dismissing the interstitial ad.
 @param interstitialAd A reference to an object of the YMAInterstitialAd class that invoked the method.
 */
- (void)interstitialAdDidDismiss:(YMAInterstitialAd *)interstitialAd {
    logDebug("interstitialAdDidDismiss");
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::InterstitialClosed);
}

/**
 Notifies that the user has clicked on the ad.
 @param interstitialAd A reference to an object of the YMAInterstitialAd class that invoked the method.
 */
- (void)interstitialAdDidClick:(YMAInterstitialAd *)interstitialAd {
    logDebug("interstitialAdDidClick");
}

/**
 Notifies delegate when an impression was tracked.
 @param interstitialAd A reference to an object of the YMAInterstitialAd class that invoked the method.
 @param impressionData Ad impression-level revenue data.
 */
- (void)interstitialAd:(YMAInterstitialAd *)interstitialAd
didTrackImpressionWithData:(nullable id<YMAImpressionData>)impressionData {
    logDebug("didTrackImpressionWithData");
}
@end

/*
    REWARDED CALLBACKS
*/
@interface iYandexRewardedVideo : NSObject
+(iYandexRewardedVideo*) defaultRewardedVideo;
@end

@interface iYandexRewardedVideo() <YMARewardedAdLoaderDelegate, YMARewardedAdDelegate>
@property(nonatomic, strong) YMARewardedAd *rewardedAd;
@property(nonatomic, strong) YMARewardedAdLoader *loader;
@property(nonatomic, strong) YMAAdRequestConfiguration* cfg;
-(void)Init;
-(void)SetUintId:(const char* )ID;
-(void)Load;
-(void)Show;
@end

@implementation iYandexRewardedVideo

+ (iYandexRewardedVideo *) defaultRewardedVideo
{
    static iYandexRewardedVideo* yandex;
    if (!yandex) {
        yandex = [[iYandexRewardedVideo alloc] init];
        [yandex Init];
    }
    return yandex;
}

-(void)Init {
    self.loader = [[YMARewardedAdLoader alloc] init];
    self.loader.delegate = self;
};

-(void)SetUintId:(const char* )IDstr {
    try {
        NSString *ID = [[NSString alloc] initWithUTF8String:IDstr];
        if(ID) {
            logDebug("RewardedVideo SetUintId");
            self.cfg = [[YMAAdRequestConfiguration alloc] initWithAdUnitID:ID];
        }
    }
    catch (NSException * ex) {
        logDebug("RewardedVideo SetUintId error");
    }
};

- (void)Load {
    logDebug("RewardedVideo Load");
    if(!ads::Yandex::iOSProvider::wasInited || !self.cfg || !self.loader) {
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToLoad);
        return;
    }

    try {
        [self.loader loadAdWithRequestConfiguration: self.cfg];
    }
    catch (NSException * ex) {
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToLoad);
    }
}

-(void)Show {
    logDebug("RewardedVideo Show");
    try {
        if(ads::Yandex::iOSProvider::wasInited && self.rewardedAd) {
            auto controller = iosWrapper::getRootViewController();
            if(controller) {
                self.rewardedAd.delegate = self;
                [self.rewardedAd showFromViewController : controller];
                return;
            }
        }
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToShow);
    }
    catch (NSException * ex) {
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToShow);
    }
}

/**
 Notifies that the ad loaded successfully.
 @param adLoader A reference to an object of the YMARewardedAdLoader class that invoked the method.
 @param rewardedAd A reference to an object of the YMARewardedAd class that invoked the method.
 */
- (void)rewardedAdLoader:(YMARewardedAdLoader *)adLoader didLoad:(YMARewardedAd *)rewardedAdID {
    logDebug("RewardedVideoLoaded");
    if(rewardedAdID) {
        self.rewardedAd = rewardedAdID;
        ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoLoaded);
        return;
    }
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToLoad);
}

/**
 Notifies that the ad failed to load.
 @param adLoader A reference to an object of the YMARewardedAdLoader class that invoked the method.
 @param error Information about the error (for details, see YMAAdErrorCode).
 */
- (void)rewardedAdLoader:(YMARewardedAdLoader *)adLoader didFailToLoadWithError:(YMAAdRequestError *)error {
    logDebug("RewardedVideoFailedToLoad: %s", [[error.error localizedDescription] UTF8String]);
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToLoad);
}

/**
 Notifies that rewarded ad has rewarded the user.
 @param rewardedAd A reference to an object of the YMARewardedAd class that invoked the method.
 @param reward Reward given to the user.
 */
- (void)rewardedAd:(YMARewardedAd *)rewardedAd didReward:(id<YMAReward>)reward {
    logDebug("RewardedVideoRewarded");
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoRewarded);
}

//@optional

/**
 Notifies that the ad can't be displayed.
 @param rewardedAd A reference to an object of the YMARewardedAd class that invoked the method.
 @param error Information about the error (for details, see YMAAdErrorCode).
 */
- (void)rewardedAd:(YMARewardedAd *)rewardedAd didFailToShowWithError:(NSError *)error {
    logDebug("RewardedVideoFailedToShow: %s", [[error localizedDescription] UTF8String]);
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoFailedToShow);
}

/**
 Called after the rewarded ad shows.
 @param rewardedAd A reference to an object of the YMARewardedAd class that invoked the method.
 */
- (void)rewardedAdDidShow:(YMARewardedAd *)rewardedAd {
    logDebug("RewardedVideoOpened");
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoOpened);
}

/**
 Called after dismissing the rewarded ad.
 @param rewardedAd A reference to an object of the YMARewardedAd class that invoked the method.
 */
- (void)rewardedAdDidDismiss:(YMARewardedAd *)rewardedAd {
    logDebug("RewardedVideoClosed");
    ads::Yandex::iOSProvider::onAdEvent(ads::Event::RewardedVideoClosed);
}

/**
 Notifies that the user has clicked on the ad.
 @param rewardedAd A reference to an object of the YMARewardedAd class that invoked the method.
 */
- (void)rewardedAdDidClick:(YMARewardedAd *)rewardedAd {
    logDebug("rewardedAdDidClick");
}

/**
 Notifies delegate when an impression was tracked.
 @param rewardedAd A reference to an object of the YMARewardedAd class that invoked the method.
 @param impressionData Ad impression-level revenue data.
 */
- (void)rewardedAd:(YMARewardedAd *)rewardedAd didTrackImpressionWithData:(nullable id<YMAImpressionData>)impressionData {
    logDebug("didTrackImpressionWithData");
}
@end

ads::Yandex::iOSProvider::iOSProvider(std::weak_ptr<ads::ProviderCallback> cbk, ads::Format format) :
ads::Yandex::Provider (cbk) {
    [YMAMobileAds initializeSDKWithCompletionHandler:^{
        logDebug("initializeSDKWithCompletionHandler success");
        wasInited = true;
        interstitialStatus = ads::InterstitialStatus::ReadyToLoad;
        rewardedVideoStatus = ads::RewardedVideoStatus::ReadyToLoad;
    }];

    // почему-то сейчас колбек не вызывается, поэтому ставим признак инициализациия явно
    // формально документация обещает автоматическую инициализацию, при вызове load рекламы
    // поэтому ставим явно здесь
    wasInited = true;
    interstitialStatus = ads::InterstitialStatus::ReadyToLoad;
    rewardedVideoStatus = ads::RewardedVideoStatus::ReadyToLoad;
};

void ads::Yandex::iOSProvider::InterstitialSetUnitId(const std::string& unitId) {
    logDebug("InterstitialSetUnitId %s", unitId.c_str());
    [[iYandexInterstitial defaultInterstitial] SetUintId: unitId.c_str()];
}

void ads::Yandex::iOSProvider::setRewardedVideoUnit(const char* unitId) {
    logDebug("setRewardedVideoUnit %s", unitId);
    [[iYandexRewardedVideo defaultRewardedVideo] SetUintId: unitId];
}

void ads::Yandex::iOSProvider::v_tryLoadInterstitial() {
    logDebug("v_tryLoadInterstitial");
    [[iYandexInterstitial defaultInterstitial] Load];
}

void ads::Yandex::iOSProvider::v_tryLoadRewardedVideo() {
    logDebug("v_tryLoadRewardedVideo");
    [[iYandexRewardedVideo defaultRewardedVideo] Load];
}

void ads::Yandex::iOSProvider::v_showInterstitial() {
    logDebug("v_showInterstitial");
    [[iYandexInterstitial defaultInterstitial] Show];
}

void ads::Yandex::iOSProvider::v_showRewardedVideo() {
    logDebug("v_showRewardedVideo");
    [[iYandexRewardedVideo defaultRewardedVideo] Show];
}
