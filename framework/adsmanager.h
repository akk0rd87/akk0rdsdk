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
	std::vector<AdMobUnit> AdMobUnits;
	
	Uint32                            ShowDelay        = 3 * 60; // 3 минуты  секундах
	Uint32                            LoadDelay        = 3;      // 3 секунды
	Uint32                            LastShowed       = 0;
	Uint32                            AdMobLastLoad    = 0;
	std::vector<AdMobUnit>::size_type currentAdmobUnit = std::numeric_limits<std::vector<AdMobUnit>::size_type>::max();

	// время в секундах с момента старта
	Uint32 GetSeconds();

	Uint32 GetInterstitialNextShowTime();
	
	// выбираем подходящий по времени Unit
	void ChooseAdmobAdBlock();    
public:
	
    void Init();
    void Clear();
	void SetIntersitialShowDelay(Uint32 DelaySeconds);
	void SetIntersitialLoadDelay(Uint32 DelaySeconds);
	void AddAdMobUnit(const std::string& Id, Uint32 TimePriority);
	bool InterstitialLoad();
	bool InterstitialShow();
	void ShowAdUnits();
};

#endif // __AKK0RD_ADS_H__