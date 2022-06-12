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
                v->preCheckInterstitialLoadStatus(currentTime);
                if (needToLoad) {
                    if (v->isReadyShowInterstitial()) { // если какой-то провайдер готов показать рекламу, остальные не опрашиваем
                        needToLoad = false;
                        continue;
                    }
                    v->tryLoadInterstitial(currentTime, nextTime);
                }
            }
        }

        void tryLoadRewardedVideo() {
            const auto currentTime = GetSeconds();
            bool needToLoad = true;
            for (auto& v : providers) {
                v->preCheckRewardedVideoLoadStatus(currentTime);
                if (needToLoad) {
                    if (v->isReadyShowRewardedVideo()) { // если какой-то провайдер готов показать рекламу, остальные не опрашиваем
                        needToLoad = false;
                        continue;
                    }
                    v->tryLoadRewardedVideo(currentTime);
                }
            }
        }

        bool isReadyShowInterstitial() const {
            if (isInterstitialShowPossible()) {
                for (auto& v : providers) {
                    if (v->isReadyShowInterstitial()) {
                        return true;
                    }
                }
            }
            return false;
        }

        bool isReadyShowRewardedVideo() const {
            for (auto& v : providers) {
                if (v->isReadyShowRewardedVideo()) {
                    return true;
                }
            }
            return false;
        }

        bool showInterstitialIfAvailable() {
            if (isInterstitialShowPossible()) {
                for (auto& v : providers) {
                    if (v->isReadyShowInterstitial()) {
                        lastShowed = GetSeconds();
                        v->showInterstitial();
                        return true;
                    }
                }
            }
            return false;
        }

        bool showRewardedVideoIfAvailable() {
            for (auto& v : providers) {
                if (v->isReadyShowRewardedVideo()) {
                    v->showRewardedVideo();
                    return true;
                }
            }
            return false;
        }

        void reserveProviders(std::size_t count) { providers.reserve(count); }
        void addProvider(const std::shared_ptr<Provider>& provider) { providers.emplace_back(provider); }

        ads::Event decodeEvent(const SDL_Event& Event) const {
            return static_cast<ads::Event>(static_cast<int>((size_t)(Event.user.data1)));
        }

    private:
        bool isInterstitialShowPossible() const {
            return GetSeconds() >= GetInterstitialNextShowTime();
        }

        virtual void eventCallback(int Event/*, Provider From*/) const {
            SDL_Event sdl_Event;
            sdl_Event.user.type = SDLeventCode;
            sdl_Event.user.data1 = (void*)(uintptr_t)Event;
            SDL_PushEvent(&sdl_Event);
        };

        const decltype(SDL_RegisterEvents(1)) SDLeventCode;
        timeMS inited, lastShowed, showDelay;
        std::vector<std::shared_ptr<Provider>> providers;
    };
};
#endif