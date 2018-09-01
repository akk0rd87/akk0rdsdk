#include <atomic>
#include "admob.h"

// Класс, в котором хранится состояние статусов рекламных блоков
class AdContextClass
{
private:
    std::atomic<int> InterstitialStatus;
    std::atomic<int> RewardedVideoStatus;
	Uint32           AdMobEventCode;
public:    
    AdMob::InterstitialStatus InterstitialGetStatus()
    {
        return (AdMob::InterstitialStatus)(InterstitialStatus.load());
    }

    void InterstitialSetStatus(AdMob::InterstitialStatus Status)
    {
        InterstitialStatus.store((int)Status);
    }

    AdMob::RewardedVideoStatus RewardedVideoGetStatus()
    {
        return (AdMob::RewardedVideoStatus)(RewardedVideoStatus.load());
    }

    void RewardedVideoSetStatus(AdMob::RewardedVideoStatus Status)
    {
        RewardedVideoStatus.store((int)Status);
    }

    AdContextClass()
    {        
        InterstitialSetStatus(AdMob::InterstitialStatus::NotInited);
        RewardedVideoSetStatus(AdMob::RewardedVideoStatus::NotInited);
    }

	Uint32 GetAdMobEventCode(){ return AdMobEventCode; }
	void   SetAdMobEventCode(){ AdMobEventCode = SDL_RegisterEvents(1); }
};

static AdContextClass AdContext;

struct AdEvent
{    
    int AdFormat;
    int EventType, Code;
};

// Разбор Interstitial Event-а
bool AdMob_ProcessInterstitialAdEvent(AdEvent* Event)
{
    auto EventType = (AdMob::InterstitialEvent)Event->EventType;
    switch (EventType)
    {
        case AdMob::InterstitialEvent::Closed:
        case AdMob::InterstitialEvent::Failed:			
        case AdMob::InterstitialEvent::LeftApplication:		
            AdContext.InterstitialSetStatus(AdMob::InterstitialStatus::Inited);
			//logDebug("Interstitial Inited");
            return true;

        case AdMob::InterstitialEvent::Loaded:
            AdContext.InterstitialSetStatus(AdMob::InterstitialStatus::Loaded);
			//logDebug("Interstitial Loaded");
            return true;
            break;

        case AdMob::InterstitialEvent::Opened:
            AdContext.InterstitialSetStatus(AdMob::InterstitialStatus::Opened);
			//logDebug("Interstitial Opened");
            return true;
            break;

        default:
			logDebug("Interstitial Something else");
			return false;
            break;
    }
    return false;
}

// Разбор Interstitial Event-а
bool AdMob_ProcessRewardedVideoAdEvent(AdEvent* Event)
{
    auto EventType = (AdMob::RewardedVideoEvent)Event->EventType;
    
    switch (EventType)
    {
        case AdMob::RewardedVideoEvent::Closed:
        case AdMob::RewardedVideoEvent::Failed:
        case AdMob::RewardedVideoEvent::LeftApplication:
        case AdMob::RewardedVideoEvent::Rewarded:
            AdContext.RewardedVideoSetStatus(AdMob::RewardedVideoStatus::Inited);
            return true;
            break;

        case AdMob::RewardedVideoEvent::Loaded:
            AdContext.RewardedVideoSetStatus(AdMob::RewardedVideoStatus::Loaded);
            return true;
            break;

        case AdMob::RewardedVideoEvent::Opened:
            AdContext.RewardedVideoSetStatus(AdMob::RewardedVideoStatus::Opened);
            return true;
            break;

        case AdMob::RewardedVideoEvent::Started:
            AdContext.RewardedVideoSetStatus(AdMob::RewardedVideoStatus::Started);
            return true;
            break;

        default:
            return false;
            break;
    }

    return false;
}

// Разбор Общего Event-а
bool AdMob_ProcessAdEvent(AdEvent* Event)
{    
	switch (Event->AdFormat)
    {
        case AdMob::Format::Interstitial:
            AdMob_ProcessInterstitialAdEvent(Event);
            break;

        case AdMob::Format::RewardedVideo:
            AdMob_ProcessRewardedVideoAdEvent(Event);			
            break;

        default:
            logError("Unsupported ad format type %u", Event->AdFormat);            
            break;
    }

	{
		// Пушим евент в основной поток
		SDL_Event sdl_Event;
		sdl_Event.user.type = AdContext.GetAdMobEventCode();
		sdl_Event.user.code = (Sint32)Event->AdFormat;
		sdl_Event.user.data1 = (void*)(uintptr_t)Event->EventType;
		sdl_Event.user.data2 = (void*)(uintptr_t)Event->Code;
		SDL_PushEvent(&sdl_Event);
	}

    return false;
}

int AdMob::GetEventAdFormat(const SDL_Event& Event)
{
	int fmt = (int)Event.user.code;
	return fmt;
}

void AdMob::RewarededDecodeEvent(const SDL_Event& Event, AdMob::RewardedVideoEvent& EventType, int& Result)
{	
	EventType = (AdMob::RewardedVideoEvent)(int)(size_t)(Event.user.data1);
	Result = (int)(size_t)Event.user.data2;
};

void AdMob::InterstitialDecodeEvent(const SDL_Event& Event, AdMob::InterstitialEvent& EventType)
{
	EventType = (AdMob::InterstitialEvent)(int)(size_t)Event.user.data1;
};

AdMob::InterstitialStatus AdMob::InterstitialGetStatus()
{
    return AdContext.InterstitialGetStatus();
};

AdMob::RewardedVideoStatus  AdMob::RewardedVideoGetStatus()
{
    return AdContext.RewardedVideoGetStatus();
};

// Функция для вызова из Java. Callback
#ifdef __ANDROID__
#include "core/android/android_admob.h"
extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_AdMobAdapter_AdCallback(JNIEnv*, jclass, jint, jint, jint);
}
JNIEXPORT void JNICALL Java_org_akkord_lib_AdMobAdapter_AdCallback(JNIEnv*, jclass, jint AdType, jint EventType, jint Code)
{
    //int AdFormat = (int)AdType;
    AdEvent Event;
    Event.AdFormat = (int)AdType;
    Event.EventType = (int)EventType;
    Event.Code = (int)Code;
    //logDebug("Add callback event %d %d %d", Event.AdFormat, Event.EventType, Event.Code);
    AdMob_ProcessAdEvent(&Event);
}
#endif

#ifdef __APPLE__
#include "core/ios/ios_admob.h"
#endif

Uint32 AdMob::GetEventCode()
{
	return AdContext.GetAdMobEventCode();
}

bool AdMob::Init(const char* PublisherID, int Formats)
{
	AdContext.SetAdMobEventCode();
	bool inited = false;

#ifdef __WIN32__
	inited = true; // inited безусловно ставим Inited
#endif

#ifdef __ANDROID__
    if (AdMobAndroid::Init(PublisherID, Formats))
    {
		inited = true;
    };
#endif

#ifdef __APPLE__
    if (AdMobiOS::Init(PublisherID, Formats))
    {
		inited = true;
    };
#endif

	if (inited)
	{
		if (Formats & AdMob::Format::Interstitial)
			AdContext.InterstitialSetStatus(AdMob::InterstitialStatus::Inited);

		if (Formats & AdMob::Format::RewardedVideo)
			AdContext.RewardedVideoSetStatus(AdMob::RewardedVideoStatus::Inited);
	}

	return inited;
};

bool AdMob::InterstitialSetUnitId(const char* UnitId)
{
#ifdef __ANDROID__    
    return AdMobAndroid::InterstitialSetUnitId(UnitId);
#endif    

    return false;
};

bool AdMob::InterstitialLoad()
{   
    if (AdContext.InterstitialGetStatus() != AdMob::InterstitialStatus::Inited)
    {
        logError("Load should be requested on only in Inited status");
        return false;
    }    
    
    AdContext.InterstitialSetStatus(AdMob::InterstitialStatus::TryingToLoad);
#ifdef __ANDROID__    
    return AdMobAndroid::InterstitialLoad();
#endif

    return false;
};

bool AdMob::InterstitialShow()
{
    if (AdContext.InterstitialGetStatus() != AdMob::InterstitialStatus::Loaded)
    {
        logError("Show should be requested on only in Loaded status");
        return false;
    }

    AdContext.InterstitialSetStatus(AdMob::InterstitialStatus::TryingToShow);
#ifdef __ANDROID__    
    return AdMobAndroid::InterstitialShow();    
#endif

    return false;
};

bool AdMob::RewardedVideoSetUnitId(const char* UnitId)
{
#ifdef __ANDROID__    
    return AdMobAndroid::RewardedVideoSetUnitId(UnitId);
#endif       
    
    return false;
};

bool AdMob::RewardedVideoLoad()
{    
    if (AdContext.RewardedVideoGetStatus() != AdMob::RewardedVideoStatus::Inited)
    {
        logError("Load should be requested on only in Inited status");
        return false;
    }
    
    AdContext.RewardedVideoSetStatus(AdMob::RewardedVideoStatus::TryingToLoad);
#ifdef __ANDROID__    
    return AdMobAndroid::RewardedVideoLoad();
#endif

    return false;
};

bool AdMob::RewardedVideoShow()
{
    if (AdContext.RewardedVideoGetStatus() != AdMob::RewardedVideoStatus::Loaded)
    {
        logError("Show should be requested on only in Loaded status");
        return false;
    }

    AdContext.RewardedVideoSetStatus(AdMob::RewardedVideoStatus::TryingToShow);
#ifdef __ANDROID__    
    return AdMobAndroid::RewardedVideoShow();
#endif

    return false;
}
