#ifndef __AKK0RD_SDK_ADS_EVENT_H__
#define __AKK0RD_SDK_ADS_EVENT_H__

namespace ads {
    enum struct Event : int { // за соответствие этих кодов должны отвечат провайдеры рекламы
        InterstitialLoaded = 1,
        InterstitialOpened = 2,
        InterstitialClosed = 3,
        InterstitialFailedToLoad = 4,
        InterstitialLeftApplication = 5,
        InterstitialFailedToShow = 6,

        RewardedVideoLoaded = 101,
        RewardedVideoOpened = 102,
        RewardedVideoClosed = 103,
        RewardedVideoFailedToLoad = 104,
        RewardedVideoLeftApplication = 105,
        RewardedVideoStarted = 106,
        RewardedVideoCompleted = 107,
        RewardedVideoRewarded = 108,
        RewardedVideoFailedToShow = 109
    };
};

#endif