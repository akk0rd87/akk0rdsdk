#ifndef __AKK0RD_SDK_ADMOBWRAPPER_H__
#define __AKK0RD_SDK_ADMOBWRAPPER_H__

#include "basewrapper.h"

class AdMob
{
private:
    //static bool ProcessInterstitialAdEvent(AdEvent* Event);
    //bool AdMob::ProcessAdEvent(AdEvent* Event);
public:
    enum struct Format : int { Interstitial = 1, RewardedVideo = 2, NativeAdsAdvanced = 4, Unknown = 0 };

    enum struct InterstitialEvent : int { Loaded = 1, Opened = 2, Closed = 3, Failed = 4, LeftApplication = 5 }; // Совпадает с Java-кодами
    enum struct InterstitialStatus : int { NotInited, Inited, TryingToLoad, Loaded };

    enum struct RewardedVideoEvent : int { Loaded = 101, Opened = 102, Closed = 103, Failed = 104, LeftApplication = 105, Started = 106, Completed = 107, Rewarded = 108 }; // Совпадает с Java-кодами
    enum struct RewardedVideoStatus : int { NotInited, Inited, TryingToLoad, Loaded };

    static bool                        Init(const char* AdMobAppID, AdMob::Format Formats) { return AdMob::Init(Formats); }; // Deprecated Method
    static bool                        Init(AdMob::Format Formats);
    static bool                        InterstitialSetUnitId(const char* UnitId);
    static AdMob::InterstitialStatus   InterstitialGetStatus();
    static bool                        InterstitialLoad();
    static bool                        InterstitialShow();

    static bool                        RewardedVideoSetUnitId(const char* UnitId);
    static AdMob::RewardedVideoStatus  RewardedVideoGetStatus();
    static bool                        RewardedVideoLoad();
    static bool                        RewardedVideoShow();

    static Uint32                      GetEventCode();
    static AdMob::Format               GetEventAdFormat(const SDL_Event& Event);
    static void                        InterstitialDecodeEvent(const SDL_Event& Event, AdMob::InterstitialEvent& EventType);
    static void                        RewarededDecodeEvent(const SDL_Event& Event, AdMob::RewardedVideoEvent& EventType, int& Result);

    // FOR FRAMEWORK INTERNAL USE
    struct AdEvent
    {
        AdMob::Format AdFormat;
        int EventType, Code;
    };
    typedef bool (AdEventCallback)(const AdMob::AdEvent* Event);

    //Запрещаем создавать экземпляр класса AdMob
    AdMob() = delete;
    ~AdMob() = delete;
    AdMob(const AdMob& rhs) = delete; // Копирующий: конструктор
    AdMob(AdMob&& rhs) = delete; // Перемещающий: конструктор
    AdMob& operator= (const AdMob& rhs) = delete; // Оператор копирующего присваивания
    AdMob& operator= (AdMob&& rhs) = delete; // Оператор перемещающего присваивания
};

inline AdMob::Format operator | (AdMob::Format a, AdMob::Format b) {
    return static_cast<AdMob::Format>(static_cast<int>(a) | static_cast<int>(b));
}

inline AdMob::Format operator & (AdMob::Format a, AdMob::Format b) {
    return static_cast<AdMob::Format>(static_cast<int>(a) & static_cast<int>(b));
}

inline bool operator == (AdMob::Format a, AdMob::Format b) {
    return static_cast<int>(a) == static_cast<int>(b);
}

inline bool operator!(AdMob::Format a) {
    return (static_cast<int>(a) == 0);
}

#endif // __AKK0RD_SDK_ADMOBWRAPPER_H__