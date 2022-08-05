#ifndef __AKK0RD_SDK_ADS_MANAGER_H__
#define __AKK0RD_SDK_ADS_MANAGER_H__

#include <vector>
#include <memory>
#include "basewrapper.h"
#include "adprovider.h"
#include "adevent.h"

namespace ads {
    class Manager : public ProviderCallback {
    public:
        Manager() :
            SDLeventCode(SDL_RegisterEvents(1)),
            inited(GetSeconds()),
            lastShowed(0),
            showDelay(3 * 60) // 3 минуты в секундах
        { }

        decltype(SDL_RegisterEvents(1)) getEventCode() const { return SDLeventCode; }

        virtual timeMS GetInterstitialNextShowTime() const {
            return (lastShowed == static_cast<decltype(lastShowed)>(0) ? inited + showDelay / static_cast <decltype(showDelay)>(2) : lastShowed + showDelay);
        };

        void SetIntersitialShowDelay(timeMS DelaySeconds) {
            showDelay = DelaySeconds;
        }

        void tryLoadInterstitial() {
            const auto currentTime = GetSeconds();
            const auto nextTime = GetInterstitialNextShowTime();
            bool needToLoad = true;
            for (auto& v : providers) {
                if (v) {
                    v->preCheckInterstitialLoadStatus(currentTime); // опциональная проверка/восстановление статусов
                    if (needToLoad) {
                        if (v->isInterstitialLoaded()) { // если какой-то провайдер готов показать рекламу, остальные не опрашиваем
                            needToLoad = false;
                            continue;
                        }
                        v->tryLoadInterstitial(currentTime, nextTime);
                    }
                }
            }
        }

        void tryLoadRewardedVideo() {
            const auto currentTime = GetSeconds();
            bool needToLoad = true;
            for (auto& v : providers) {
                if (v) {
                    v->preCheckRewardedVideoLoadStatus(currentTime); // опциональная проверка/восстановление статусов
                    if (needToLoad) {
                        if (v->isRewardedVideoLoaded()) { // если какой-то провайдер готов показать рекламу, остальные не опрашиваем
                            needToLoad = false;
                            continue;
                        }
                        v->tryLoadRewardedVideo(currentTime);
                    }
                }
            }
        }

        // функция проверяет 2 факта:
        // - допустим ли показ по времени
        // - загружена ли реклама
        bool isReadyShowInterstitial() const {
            return isInterstitialShowPossible() && isInterstitialLoaded();
        }

        bool isInterstitialLoaded() const {
            for (const auto& v : providers) {
                if (v) {
                    if (v->isInterstitialLoaded()) {
                        return true;
                    }
                }
            }
            return false;
        }

        bool isRewardedVideoLoaded() const {
            for (const auto& v : providers) {
                if (v) {
                    if (v->isRewardedVideoLoaded()) {
                        return true;
                    }
                }
            }
            return false;
        }

        bool showInterstitialIfAvailable() {
            if (isInterstitialShowPossible()) { // проверка на допустимость показа по времени
                for (auto& v : providers) {
                    if (v) {
                        if (v->isInterstitialLoaded()) {
                            lastShowed = GetSeconds();
                            v->showInterstitial();
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        bool showRewardedVideoIfAvailable() {
            for (auto& v : providers) {
                if (v) {
                    if (v->isRewardedVideoLoaded()) {
                        v->showRewardedVideo();
                        return true;
                    }
                }
            }
            return false;
        }

        void reserveProviders(std::size_t count) { providers.reserve(count); }

        void addProvider(const std::shared_ptr<Provider>& provider) {
            if (provider) {
                providers.emplace_back(provider);
            }
        }

        ads::Event decodeEvent(const SDL_Event& Event) const {
            return static_cast<ads::Event>(static_cast<int>((size_t)(Event.user.data1)));
        }

    private:
        // функция определяет только допустимость показа в соответствии со временем
        bool isInterstitialShowPossible() const {
            return GetSeconds() >= GetInterstitialNextShowTime();
        }

        virtual void eventCallback(ads::Event Event/*, Provider From*/) const {
            SDL_Event sdl_Event;
            sdl_Event.user.type = SDLeventCode;
            sdl_Event.user.data1 = (void*)(uintptr_t)static_cast<int>(Event);
            SDL_PushEvent(&sdl_Event);
        };

        const decltype(SDL_RegisterEvents(1)) SDLeventCode;
        timeMS inited, lastShowed, showDelay;
        std::vector<std::shared_ptr<Provider>> providers;
    };
};
#endif