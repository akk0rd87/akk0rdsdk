#ifndef __AKK0RD_SDK_ADS_PROVIDER_H__
#define __AKK0RD_SDK_ADS_PROVIDER_H__

#include "adprovidercallback.h"
#include "adformat.h"

namespace ads {
    using timeMS = decltype(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    class Provider {
    public:
        Provider(std::weak_ptr<ProviderCallback> callback, timeMS interstitialLoadDelaySeconds, timeMS rewardedVideoLoadDelaySeconds) :
            callback(callback),
            lastIntLoadTime(0),
            lastRewLoadTime(0),
            interstitialStatus(InterstitialStatus::NotInited),
            rewardedVideoStatus(RewardedVideoStatus::NotInited),
            interstitialLoadDelay(interstitialLoadDelaySeconds), // секунды интервала между запросами загрузки рекламы
            rewardedVideoLoadDelay(rewardedVideoLoadDelaySeconds) // секунды интервала между запросами загрузки рекламы
        {}

        void tryLoadInterstitial(timeMS currentTime, timeMS nextShowTime) {
            if (InterstitialStatus::ReadyToLoad == interstitialStatus && lastIntLoadTime + interstitialLoadDelay < currentTime && isInited()) {
                interstitialStatus = InterstitialStatus::Loading;
                lastIntLoadTime = currentTime;
                v_tryLoadInterstitial(currentTime, nextShowTime);
            }
        }

        void tryLoadRewardedVideo(timeMS currentTime) {
            if (RewardedVideoStatus::ReadyToLoad == rewardedVideoStatus && lastRewLoadTime + rewardedVideoLoadDelay < currentTime && isInited()) {
                rewardedVideoStatus = RewardedVideoStatus::Loading;
                lastRewLoadTime = currentTime;
                v_tryLoadRewardedVideo();
            }
        }

        bool isReadyShowInterstitial() const { return InterstitialStatus::Loaded == interstitialStatus; }
        bool isReadyShowRewardedVideo() const { return RewardedVideoStatus::Loaded == rewardedVideoStatus; }

        void showInterstitial() {
            if (isReadyShowInterstitial()) {
                interstitialStatus = InterstitialStatus::Showing;
                v_showInterstitial();
            }
        }

        void showRewardedVideo() {
            if (isReadyShowRewardedVideo()) {
                rewardedVideoStatus = RewardedVideoStatus::Showing;
                v_showRewardedVideo();
            }
        }

        // эти функции нужны на случай кастомизации поведения загрузки рекламы
        virtual void preCheckInterstitialLoadStatus(timeMS currentTime) {}
        virtual void preCheckRewardedVideoLoadStatus(timeMS currentTime) {}

        virtual ~Provider() = default;
    private:
        std::weak_ptr<ProviderCallback> callback;
        timeMS lastIntLoadTime, lastRewLoadTime;

    protected:
        enum struct InterstitialStatus : uint8_t { NotInited, ReadyToLoad, Loading, LoadError, Loaded, Showing };
        enum struct RewardedVideoStatus : uint8_t { NotInited, ReadyToLoad, Loading, LoadError, Loaded, Showing };

        InterstitialStatus interstitialStatus;
        RewardedVideoStatus rewardedVideoStatus;
        const ads::timeMS interstitialLoadDelay; // секунды интервала между запросами загрузки рекламы
        const ads::timeMS rewardedVideoLoadDelay; // секунды интервала между запросами загрузки рекламы

        virtual bool isInited() const { return false; }
        virtual void v_tryLoadInterstitial(timeMS currentTime, timeMS nextShowTime) {}
        virtual void v_tryLoadRewardedVideo() {}
        virtual void v_showInterstitial() {}
        virtual void v_showRewardedVideo() {}

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