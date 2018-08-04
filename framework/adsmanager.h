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
	
	// список AdUnit-ов
	static std::vector<AdMobUnit> AdMobUnits;
	
	const Uint32 ShowDelay     = 3 * 60; // 3 минуты  секундах
	const Uint32 LoadDelay     = 3;      // 3 секунды
	Uint32       LastShowed    = 0;
	Uint32       AdMobLastLoad = 0;

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
	
	void ChooseAdmobAdBlock()
	{

	};

public:
	
	void AddAdMobUnit(const std::string& Id, Uint32 TimePriority)
	{		
		AdMobUnits.emplace_back(AdMobUnit(Id, TimePriority));
	}

	void AddMobInterstitialLoad()
	{
		auto NewTm = GetSeconds(); // время в секундах

		if (NewTm - AdMobLastLoad > LoadDelay || 0 == AdMobLastLoad)
			if (AdMob::InterstitialGetStatus() == AdMob::InterstitialStatus::Inited)
			{
				ChooseAdmobAdBlock();				
				AdMob::InterstitialLoad();
				AdMobLastLoad = NewTm;
			}
	};
};

#endif // __AKK0RD_ADS_H__