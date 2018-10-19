#pragma once
#ifndef __AKK0RD_IOS_ADMOB_H__
#define __AKK0RD_IOS_ADMOB_H__
#include "basewrapper.h"
#include "admob.h"
#include "../core_defines.h"

class AdMobiOS
{
public:
    static bool Init(const char* AdMobAppID, int Formats);
    
    static bool InterstitialSetUnitId(const char* UnitId);
    static bool InterstitialLoad();
    static bool InterstitialShow();
    
    static bool RewardedVideoSetUnitId(const char* UnitId);
    static bool RewardedVideoLoad();
    static bool RewardedVideoShow();
    
    static AdMob::InterstitialStatus   InterstitialGetStatus();
};

#endif // __AKK0RD_IOS_ADMOB_H__
