#ifndef __AKK0RD_SDK_ADS_ADMOB_PROVIDER_H__
#define __AKK0RD_SDK_ADS_ADMOB_PROVIDER_H__

#include <vector>
#include <string>
#include "ads/adprovider.h"

namespace ads {
    namespace AdMob {
        class Provider : public ads::Provider {
        public:
            Provider(std::weak_ptr<ads::ProviderCallback> Callback) :
                ads::Provider(Callback, 5, 5),
                currentAdmobUnit(std::numeric_limits<std::vector<Unit>::size_type>::max())
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
        protected:
            virtual void InterstitialSetUnitId(const std::string& unitId) {}
            virtual void setRewardedVideoUnitId(const char* UnitId) { }
            virtual void admob_tryLoadInterstitial() {}
            virtual void admob_tryLoadRewardedVideo() {}

            virtual void v_tryLoadRewardedVideo() override {
                admob_tryLoadRewardedVideo();
            }

            virtual void v_tryLoadInterstitial(timeMS currentTime, timeMS nextShowTime) override {
                ChooseAdmobAdBlock(currentTime, nextShowTime);
                admob_tryLoadInterstitial();
            }

            // выбираем подходящий по времени Unit
            void ChooseAdmobAdBlock(timeMS currentTime, timeMS nextShowTime) {
                auto searchUnit = std::numeric_limits<std::vector<Unit>::size_type>::max();

                // Если новое время еще наступило
                if (nextShowTime > currentTime) {
                    const auto RemainingTime = nextShowTime - currentTime;

                    // ищем подходящий, время должно быть отсортировано по возрастанию
                    for (decltype(interstitialUnits.size())i = 0; i < interstitialUnits.size(); ++i)
                        if (RemainingTime <= interstitialUnits[i].timePriority) {
                            searchUnit = i;
                            break;
                        }

                    // Если не нашли, берем последний
                    if (searchUnit == std::numeric_limits<decltype(interstitialUnits.size())>::max())
                        searchUnit = interstitialUnits.size() - 1;
                }
                else { // если просрочили
                    // берем первый, с самым низким приоритетом
                    searchUnit = 0;
                }

                //logDebug("(currentAdmobUnit = %u; searchUnit = %u", currentAdmobUnit, searchUnit);
                if (currentAdmobUnit != searchUnit) {
                    currentAdmobUnit = searchUnit;
                    InterstitialSetUnitId(interstitialUnits[currentAdmobUnit].id);
                }
            }

            // структура данных, которая хранит id блока и время, когда на него переключаться
            struct Unit {
                std::string id;           // id блока
                timeMS      timePriority; // время, когда на него переключаться
                Unit(const std::string& Id, timeMS TimePriority) :id(Id), timePriority(TimePriority) {};
            };

            std::vector<Unit> interstitialUnits;
            std::vector<Unit>::size_type currentAdmobUnit;
        };
    };
};
#endif