#ifndef __AKK0RD_ADS_MANAGER__
#define __AKK0RD_ADS_MANAGER__

#include <chrono>
#include "admob.h"

class adsManager
{
    using timeMS = decltype(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    static auto getTicks() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    };

    // структура данных, которая хранит id блока и время, когда на него переключаться
    struct AdMobUnit
    {
        std::string Id;           // id блока
        timeMS      TimePriority; // время, когда на него переключаться
        AdMobUnit(const std::string& Id, timeMS TimePriority) :Id(Id), TimePriority(TimePriority) {};
    };

    // список AdUnit-ов
    std::vector<AdMobUnit> AdMobUnits;

    timeMS ShowDelay, LoadDelay, Inited, LastShowed, AdMobLastLoad;
    std::vector<AdMobUnit>::size_type currentAdmobUnit;

    // время в секундах с начала эры
    timeMS GetSeconds() { return adsManager::getTicks() / 1000; };

    timeMS GetInterstitialNextShowTime() {
        return (LastShowed == static_cast<decltype(LastShowed)>(0) ? Inited + ShowDelay / static_cast <decltype(ShowDelay)>(2) : LastShowed + ShowDelay);
    };

    // выбираем подходящий по времени Unit
    void ChooseAdmobAdBlock();
public:

    void Init()
    {
        this->AdMobUnits.clear();
        this->Inited = GetSeconds();
        this->LastShowed = static_cast<decltype(LastShowed)>(0);
        this->AdMobLastLoad = static_cast<decltype(AdMobLastLoad)>(0);
        this->currentAdmobUnit = std::numeric_limits<std::vector<AdMobUnit>::size_type>::max();
        this->ShowDelay = 3 * 60; // 3 минуты в секундах
        this->LoadDelay = 3;      // 3 секунды
    };

    void Clear() { AdMobUnits.clear(); };
    void SetIntersitialShowDelay(timeMS DelaySeconds) { this->ShowDelay = DelaySeconds; };
    void SetIntersitialLoadDelay(timeMS DelaySeconds) { this->LoadDelay = DelaySeconds; };
    void AddAdMobUnit(const std::string& Id, timeMS TimePriority) { AdMobUnits.emplace_back(AdMobUnit(Id, TimePriority)); };
    bool InterstitialLoad();
    bool InterstitialShow();
    void ShowAdUnits();

    adsManager() {};
    ~adsManager() { Clear(); };

    //Запрещаем создавать экземпляр класса adsManager
    adsManager(const adsManager& rhs) = delete; // Копирующий: конструктор
    adsManager(adsManager&& rhs) = delete; // Перемещающий: конструктор
    adsManager& operator= (const adsManager& rhs) = delete; // Оператор копирующего присваивания
    adsManager& operator= (adsManager&& rhs) = delete; // Оператор перемещающего присваивания
};

#endif // __AKK0RD_ADS_MANAGER__