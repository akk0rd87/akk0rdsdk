#ifndef __AKK0RD_ADS_H__
#define __AKK0RD_ADS_H__

#include "admob.h"

class adsManager
{	
	// структура данных, которая хранит id блока и время, когда на него переключаться
	struct AdMobUnit
	{
		std::string Id;           // id блока
		Uint32      TimePriority; // время, когда на него переключаться
		AdMobUnit(const std::string& Id, Uint32 TimePriority) :Id(Id), TimePriority(TimePriority){}
	};

	//using s_type = 
	
	// список AdUnit-ов
	std::vector<AdMobUnit> AdMobUnits;
	
	Uint32                            ShowDelay        = 3 * 60; // 3 минуты  секундах
	Uint32                            LoadDelay        = 3;      // 3 секунды
	Uint32                            LastShowed       = 0;
	Uint32                            AdMobLastLoad    = 0;
	std::vector<AdMobUnit>::size_type currentAdmobUnit = std::numeric_limits<std::vector<AdMobUnit>::size_type>::max();

	// время в секундах с момента старта
	Uint32 GetSeconds()
	{
		return BWrapper::GetTicks() / 1000;
	}

	Uint32 GetInterstitialNextShowTime()
	{
		if (LastShowed == 0) return ShowDelay / 2;
		else                 return LastShowed + ShowDelay;
	}
	
	// выбираем подходящий по времени Unit
	void ChooseAdmobAdBlock()
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

public:
	
	void Init()
	{
		AdMobUnits.clear();
		currentAdmobUnit = std::numeric_limits<std::vector<AdMobUnit>::size_type>::max();
	}
	
	void SetIntersitialShowDelay(Uint32 DelaySeconds)
	{
		this->ShowDelay = DelaySeconds;
	};

	void SetIntersitialLoadDelay(Uint32 DelaySeconds)
	{
		this->LoadDelay = DelaySeconds;
	};

	void AddAdMobUnit(const std::string& Id, Uint32 TimePriority)
	{		
		AdMobUnits.emplace_back(AdMobUnit(Id, TimePriority));
	}

	bool InterstitialLoad()
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

	bool InterstitialShow()
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

	void ShowAdUnits()
	{
		for (auto &v : AdMobUnits)
			logDebug("Ad Unit %s %u", v.Id.c_str(), v.TimePriority);
	}
};

#endif // __AKK0RD_ADS_H__