#include "adsmanager.h"

// время в секундах с момента старта
Uint32 adsManager::GetSeconds()
{
    return BWrapper::GetTicks() / 1000;
}

Uint32 adsManager::GetInterstitialNextShowTime()
{
    if (LastShowed == 0) return ShowDelay / 2;
    else                 return LastShowed + ShowDelay;
}

// выбираем подходящий по времени Unit
void adsManager::ChooseAdmobAdBlock()
{
    auto CurrentTm = GetSeconds();
    auto NextTm    = GetInterstitialNextShowTime();

    decltype(AdMobUnits.size()) searchUnit = std::numeric_limits<std::vector<AdMobUnit>::size_type>::max();

    // Если новое время еще наступило
    if (NextTm > CurrentTm)
    {
        auto RemainingTime = NextTm - CurrentTm;

        // ищем подходящий, время должно быть отсортировано по возрастанию
        for (decltype(AdMobUnits.size())i = 0; i < AdMobUnits.size(); ++i)
            if (RemainingTime <= AdMobUnits[i].TimePriority)
            {
                searchUnit = i;
                break;
            }

        // Если не нашли, берем последний
        if (searchUnit == std::numeric_limits<decltype(AdMobUnits.size())>::max())
            searchUnit = AdMobUnits.size() - 1;
    }
    else // если просрочили
    {
        // берем первый, с самым низким приоритетом
        searchUnit = 0;
    }

    logDebug("(currentAdmobUnit = %u; searchUnit = %u", currentAdmobUnit, searchUnit);
    if (currentAdmobUnit != searchUnit)
    {
        currentAdmobUnit = searchUnit;			
        logDebug("set adUnit = %s",  AdMobUnits[currentAdmobUnit].Id.c_str());
        AdMob::InterstitialSetUnitId(AdMobUnits[currentAdmobUnit].Id.c_str());			
    }
};

void adsManager::Init()
{
    AdMobUnits.clear();
    currentAdmobUnit = std::numeric_limits<std::vector<AdMobUnit>::size_type>::max();
}

void adsManager::SetIntersitialShowDelay(Uint32 DelaySeconds)
{
    this->ShowDelay = DelaySeconds;
};

void adsManager::SetIntersitialLoadDelay(Uint32 DelaySeconds)
{
    this->LoadDelay = DelaySeconds;
};

void adsManager::AddAdMobUnit(const std::string& Id, Uint32 TimePriority)
{		
    AdMobUnits.emplace_back(AdMobUnit(Id, TimePriority));
}

bool adsManager::InterstitialLoad()
{
    if (AdMob::InterstitialGetStatus() == AdMob::InterstitialStatus::Inited)
    {
        //logDebug("Timing current = %u, LastLoad = %u", GetSeconds(), AdMobLastLoad);
        auto NewTm = GetSeconds(); // время в секундах
        if (NewTm - AdMobLastLoad > LoadDelay || 0 == AdMobLastLoad)
        {
            ChooseAdmobAdBlock();
            AdMob::InterstitialLoad();
            AdMobLastLoad = NewTm;
            return true;
        }; 
    };

    return false;
};

bool adsManager::InterstitialShow()
{
    if (AdMob::InterstitialGetStatus() == AdMob::InterstitialStatus::Loaded)
    {
        logDebug("NewTm = %u, NextShowTime = %u", GetSeconds(), GetInterstitialNextShowTime());
        auto NewTm = GetSeconds(); // время в секундах
        if (NewTm >= GetInterstitialNextShowTime())
        {
            LastShowed = NewTm;
            AdMob::InterstitialShow();
            return true;
        }
    };
    return false;
};

void adsManager::ShowAdUnits()
{
#ifdef __AKK0RD_DEBUG_MACRO__ // пришлось поставить условную компиляцию, а то в release-сборке компилятор ругается на неиспользованную переменную v
        for (auto &v : AdMobUnits)
            logDebug("Ad Unit %s %u", v.Id.c_str(), v.TimePriority);
#endif
}

void adsManager::Clear()
{
    AdMobUnits.clear();
}