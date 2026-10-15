#ifndef __AKK0RD_SDK_IOS_AD_ANALYTICS__
#define __AKK0RD_SDK_IOS_AD_ANALYTICS__

// События рекламы в Firebase Analytics — те же имена и параметры, что у Android (Utils.logFirebaseInterstitialShow,
// logFirebaseRewardedVideoShow, logFirebaseRewardedVideoRewarded). Только заголовок: адаптеры рекламы собираются и в
// приложениях без Firebase Analytics, там события просто не пишутся.

#import <Foundation/Foundation.h>
#include <string>

#if __has_include(<FirebaseAnalytics/FirebaseAnalytics.h>)
#import <FirebaseAnalytics/FirebaseAnalytics.h>
#define AKKORD_IOS_AD_ANALYTICS 1
#endif

namespace Firebase {
    namespace Analytics {
        namespace iOS {
            inline void LogAdEvent(NSString* eventName, NSString* source, const std::string& adUnitId) {
#ifdef AKKORD_IOS_AD_ANALYTICS
                NSMutableDictionary* parameters = [NSMutableDictionary dictionaryWithObject:source forKey:@"source"];
                if (!adUnitId.empty()) {
                    parameters[@"ad_unit_id"] = [NSString stringWithUTF8String:adUnitId.c_str()];
                }
                [FIRAnalytics logEventWithName:eventName parameters:parameters];
#endif
            }

            inline void LogInterstitialShow(NSString* source, const std::string& adUnitId) {
                LogAdEvent(@"interstitial_show", source, adUnitId);
            }

            inline void LogRewardedVideoShow(NSString* source, const std::string& adUnitId) {
                LogAdEvent(@"rewarded_video_show", source, adUnitId);
            }

            inline void LogRewardedVideoRewarded(NSString* source, const std::string& adUnitId) {
                LogAdEvent(@"rewarded_video_rewarded", source, adUnitId);
            }
        }
    }
}

#endif
