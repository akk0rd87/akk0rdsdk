#include "adsmanager.h"

// выбираем подходящий по времени Unit
void adsManager::ChooseAdmobAdBlock()
{
    const auto CurrentTm = GetSeconds();
    const auto NextTm = GetInterstitialNextShowTime();

    decltype(AdMobUnits.size()) searchUnit = std::numeric_limits<std::vector<AdMobUnit>::size_type>::max();

    // Если новое время еще наступило
    if (NextTm > CurrentTm)
    {
        const auto RemainingTime = NextTm - CurrentTm;

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

    //logDebug("(currentAdmobUnit = %u; searchUnit = %u", currentAdmobUnit, searchUnit);
    if (currentAdmobUnit != searchUnit)
    {
        currentAdmobUnit = searchUnit;
        //logDebug("set adUnit = %s", AdMobUnits[currentAdmobUnit].Id.c_str());
        AdMob::InterstitialSetUnitId(AdMobUnits[currentAdmobUnit].Id.c_str());
    }
};

bool adsManager::InterstitialLoad()
{
    if (AdMob::InterstitialGetStatus() == AdMob::InterstitialStatus::Inited)
    {
        //logDebug("Timing current = %u, LastLoad = %u", GetSeconds(), AdMobLastLoad);
        const auto NewTm = GetSeconds(); // время в секундах
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
    for (const auto& v : AdMobUnits)
        logDebug("Ad Unit %s %u", v.Id.c_str(), v.TimePriority);
#endif
}