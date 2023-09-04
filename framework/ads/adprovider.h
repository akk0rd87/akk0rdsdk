#ifndef __AKK0RD_SDK_ADS_PROVIDER_H__
#define __AKK0RD_SDK_ADS_PROVIDER_H__

#include "adprovidercallback.h"
#include "adformat.h"
#include "adunitstatus.h"
#include <string>

namespace ads {
    using timeMS = decltype(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    class Provider {
    public:
        Provider(std::weak_ptr<ProviderCallback> callback, timeMS interstitialLoadDelaySeconds, timeMS rewardedVideoLoadDelaySeconds) :
            callback(callback),
            lastIntLoadTime(0),
            lastRewLoadTime(0),
            currentInterstitialUnit(std::numeric_limits<std::vector<Unit>::size_type>::max()),
            interstitialStatus(InterstitialStatus::NotInited),
            rewardedVideoStatus(RewardedVideoStatus::NotInited),
            interstitialLoadDelay(interstitialLoadDelaySeconds), // секунды интервала между запросами загрузки рекламы
            rewardedVideoLoadDelay(rewardedVideoLoadDelaySeconds) // секунды интервала между запросами загрузки рекламы
        {}

        void addInterstitialUnit(const std::string& unit, timeMS timePriority) {
            addInterstitialUnit(unit.c_str(), timePriority);
        }

        virtual void addInterstitialUnit(const char* unit, timeMS timePriority) {
            interstitialUnits.emplace_back(unit, timePriority);
        }

        void reserveInterstitialUnits(std::size_t count) {
            interstitialUnits.reserve(count);
        }

        void setRewardedVideoUnit(const std::string& unit) {
            setRewardedVideoUnit(unit.c_str());
        }

        virtual void setRewardedVideoUnit(const char* unit) {
        }

        void tryLoadInterstitial(timeMS currentTime, timeMS nextShowTime) {
            if (!interstitialUnits.empty()) {
                if (InterstitialStatus::ReadyToLoad == interstitialStatus && lastIntLoadTime + interstitialLoadDelay < currentTime && isInited()) {
                    interstitialStatus = InterstitialStatus::Loading;
                    lastIntLoadTime = currentTime;
                    ChooseInterstitialBlock(currentTime, nextShowTime);
                    v_tryLoadInterstitial();
                }
            }
        }

        void tryLoadRewardedVideo(timeMS currentTime) {
            if (RewardedVideoStatus::ReadyToLoad == rewardedVideoStatus && lastRewLoadTime + rewardedVideoLoadDelay < currentTime && isInited()) {
                rewardedVideoStatus = RewardedVideoStatus::Loading;
                lastRewLoadTime = currentTime;
                v_tryLoadRewardedVideo();
            }
        }

        bool isInterstitialLoaded() const { return InterstitialStatus::Loaded == interstitialStatus; }
        bool isRewardedVideoLoaded() const { return RewardedVideoStatus::Loaded == rewardedVideoStatus; }

        void showInterstitial() {
            if (isInterstitialLoaded()) {
                interstitialStatus = InterstitialStatus::Showing;
                v_showInterstitial();
            }
        }

        void showRewardedVideo() {
            if (isRewardedVideoLoaded()) {
                rewardedVideoStatus = RewardedVideoStatus::Showing;
                v_showRewardedVideo();
            }
        }

        // эти функции нужны на случай кастомизации поведения загрузки рекламы
        // например, проверка/восстановление статусов в нестандартных ситуациях
        virtual void preCheckInterstitialLoadStatus(timeMS currentTime) {}
        virtual void preCheckRewardedVideoLoadStatus(timeMS currentTime) {}

        virtual ~Provider() = default;
    private:
        // структура данных, которая хранит id блока и время, когда на него переключаться
        struct Unit {
            std::string id;           // id блока
            timeMS      timePriority; // время, когда на него переключаться
            Unit(const std::string& Id, timeMS TimePriority) :id(Id), timePriority(TimePriority) {};
        };

        std::weak_ptr<ProviderCallback> callback;
        timeMS lastIntLoadTime, lastRewLoadTime;
        std::vector<Unit> interstitialUnits;
        std::vector<Unit>::size_type currentInterstitialUnit;

        // выбираем подходящий по времени Unit
        void ChooseInterstitialBlock(timeMS currentTime, timeMS nextShowTime) {
            auto searchUnit = std::numeric_limits<std::vector<Unit>::size_type>::max();

            // Если новое время еще наступило
            if (nextShowTime > currentTime) {
                const auto RemainingTime = nextShowTime - currentTime;

                // ищем подходящий, время должно быть отсортировано по возрастанию
                for (decltype(interstitialUnits.size())i = 0; i < interstitialUnits.size(); ++i) {
                    if (RemainingTime <= interstitialUnits[i].timePriority) {
                        searchUnit = i;
                        break;
                    }
                }

                // Если не нашли, берем последний
                if (searchUnit == std::numeric_limits<decltype(interstitialUnits.size())>::max()) {
                    searchUnit = interstitialUnits.size() - 1;
                }
            }
            else { // если просрочили
                // берем первый, с самым низким приоритетом
                searchUnit = 0;
            }

            //logDebug("(currentAdmobUnit = %u; searchUnit = %u", currentAdmobUnit, searchUnit);
            if (currentInterstitialUnit != searchUnit) {
                currentInterstitialUnit = searchUnit;
                InterstitialSetUnitId(interstitialUnits[currentInterstitialUnit].id);
            }
        }

    protected:
        InterstitialStatus interstitialStatus;
        RewardedVideoStatus rewardedVideoStatus;
        const ads::timeMS interstitialLoadDelay; // секунды интервала между запросами загрузки рекламы
        const ads::timeMS rewardedVideoLoadDelay; // секунды интервала между запросами загрузки рекламы

        virtual bool isInited() const { return false; }
        virtual void v_tryLoadInterstitial() {}
        virtual void v_tryLoadRewardedVideo() {}
        virtual void v_showInterstitial() {}
        virtual void v_showRewardedVideo() {}
        virtual void InterstitialSetUnitId(const std::string& unitId) {};

        void eventCallback(ads::Event Event) {
            switch (Event) {
            case ads::Event::InterstitialClosed:
            case ads::Event::InterstitialFailedToLoad:
            case ads::Event::InterstitialFailedToShow:
            case ads::Event::InterstitialLeftApplication:
                interstitialStatus = InterstitialStatus::ReadyToLoad;
                break;

            case ads::Event::InterstitialLoaded:
                interstitialStatus = InterstitialStatus::Loaded;
                break;

            case ads::Event::InterstitialOpened:
                interstitialStatus = InterstitialStatus::Showing;
                break;

            case ads::Event::RewardedVideoClosed:
            case ads::Event::RewardedVideoCompleted:
            case ads::Event::RewardedVideoFailedToLoad:
            case ads::Event::RewardedVideoFailedToShow:
            case ads::Event::RewardedVideoLeftApplication:
            case ads::Event::RewardedVideoRewarded:
                rewardedVideoStatus = RewardedVideoStatus::ReadyToLoad;
                break;

            case ads::Event::RewardedVideoLoaded:
                rewardedVideoStatus = RewardedVideoStatus::Loaded;
                break;

            case ads::Event::RewardedVideoOpened:
            case ads::Event::RewardedVideoStarted:
                rewardedVideoStatus = RewardedVideoStatus::Showing;
                break;
            }

            if (auto cbk = callback.lock()) {
                cbk->eventCallback(Event);
            }
        }
    };
};
#endif