#pragma once
#ifndef __AKK0RD_ADMOBWRAPPER_H__
#define __AKK0RD_ADMOBWRAPPER_H__

#include "basewrapper.h"

class AdMob
{

private:    
    //static bool ProcessInterstitialAdEvent(AdEvent* Event);    
    //bool AdMob::ProcessAdEvent(AdEvent* Event);
public:    
    struct Format { enum : int { Interstitial = 1, RewardedVideo = 2, NativeAdsAdvanced = 4, Unknown = 0 }; };

    enum struct InterstitialEvent   : int { Loaded = 1, Opened = 2, Closed = 3, Failed = 4, LeftApplication = 5 }; // Совпадает с Java-кодами
    enum struct InterstitialStatus  : int { NotInited, Inited, TryingToLoad, Loaded, TryingToShow, Opened };
    
	enum struct RewardedVideoEvent : int { Loaded = 101, Opened = 102, Closed = 103, Failed = 104, LeftApplication = 105, Started = 106, Completed = 107, Rewarded = 108 }; // Совпадает с Java-кодами
    enum struct RewardedVideoStatus : int { NotInited, Inited, TryingToLoad, Loaded, TryingToShow, Opened, Started };

    static bool                        Init(const char* AdMobAppID, int Formats);    
    static bool                        InterstitialSetUnitId(const char* UnitId);
    static AdMob::InterstitialStatus   InterstitialGetStatus();
    static bool                        InterstitialLoad();
    static bool                        InterstitialShow();

    static bool                        RewardedVideoSetUnitId(const char* UnitId);
    static AdMob::RewardedVideoStatus  RewardedVideoGetStatus();
    static bool                        RewardedVideoLoad();
    static bool                        RewardedVideoShow();
	
	static Uint32                      GetEventCode();
	static int                         GetEventAdFormat       (const SDL_Event& Event);
	static void                        InterstitialDecodeEvent(const SDL_Event& Event, AdMob::InterstitialEvent& EventType);
	static void                        RewarededDecodeEvent   (const SDL_Event& Event, AdMob::RewardedVideoEvent& EventType, int& Result);
	
    
    // FOR FRAMEWORK INTERNAL USE
    struct AdEvent
    {    
        int AdFormat;
        int EventType, Code;
    };    
    typedef bool (AdEventCallback)(const AdEvent* Event);
};

#endif // __AKK0RD_ADMOBWRAPPER_H__