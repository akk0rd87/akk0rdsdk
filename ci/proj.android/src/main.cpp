#include "basewrapper.h"
#include "videodriver.h"
#include "billing.h"
#include "firebase/analytics/firebaseanalytics.h"
#include "gdpr/gdpr_consentpolicymanager.h"
#include "ads/admanager.h"
#include "ads/admob/admobfactory.h"
#include "ads/yandex/yandexadsfactory.h"

bool Init()
{
    switch (BWrapper::GetDeviceOS())
    {
    case BWrapper::OS::Windows: // для винды работаем на openGL 2.1
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        break;

    default: // на всем остальном работаем на openGLES 2.0
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
        break;
    }

    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    if (!BWrapper::Init(SDL_INIT_VIDEO)) {
        logError("Init Error");
        return false;
    }

    auto Window = BWrapper::CreateRenderWindow("JCross Nonograms", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | (BWrapper::GetDeviceOS() == BWrapper::OS::Windows ? 0 : SDL_WINDOW_BORDERLESS));
    if (!Window) {
        logError("Window Create Error %s", SDL_GetError());
        return false;
    }

    BWrapper::SetActiveWindow(Window);

    auto Renderer = BWrapper::CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);
    if (!Renderer) {
        logError("Renderer Create Error %s", SDL_GetError());
        return false;
    }

    BWrapper::SetActiveRenderer(Renderer);
    SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);
    VideoDriver::Init(VideoDriver::Feature::SDF);
    BWrapper::SetLogPriority(BWrapper::LogPriority::Debug);
    BWrapper::GetLogParams()->showFunction = false;
    return true;
}

std::shared_ptr<ads::Manager> getAdsManager() {
    auto adsMgr = std::make_shared<ads::Manager>();
    adsMgr->SetIntersitialShowDelay(100);

    if (auto adMob = ads::AdMob::createProvider(adsMgr, ads::Format::Interstitial)) {
        adsMgr->addProvider(adMob);
    }

    if (auto yandex = ads::Yandex::createProvider(adsMgr, ads::Format::Interstitial)) {
        adsMgr->addProvider(yandex);
    }

    return adsMgr;
}

class Application : public BillingCallbackObserver {
    virtual void PurchaseUpdatedCallback(const char* PurchaseToken, const char* ProductSKU, BillingManager::OperAction Action) override { // interface of BillingCallbackObserver
    }

    virtual void PurchaseConsumedCallback(const char* PurchaseToken, const char* ProductSKU) override {// interface of BillingCallbackObserver
    }
};

int main(int, char**) {
    if (Init()) {
        Firebase::Analytics::Init();
        const auto gdprConsentPolicyGatherEventCode = SDL_RegisterEvents(1);
        const auto gdprPrivacyOptionsRequiredEventCode = SDL_RegisterEvents(1);
        const auto msgBoxEventCode = msgBox::GetEventCode();
        const auto billingEventCode = BillingManager::GetEventCode();

        GDPRConsentPolicy::getManagerInstance().initialize(
            [gdprConsentPolicyGatherEventCode]() {
                SDL_Event sdl_Event;
                sdl_Event.user.type = gdprConsentPolicyGatherEventCode;
                SDL_PushEvent(&sdl_Event);
            },
            [gdprPrivacyOptionsRequiredEventCode]() {
                SDL_Event sdl_Event;
                sdl_Event.user.type = gdprPrivacyOptionsRequiredEventCode;
                SDL_PushEvent(&sdl_Event);
            }
        );

        Application app;
        BillingManager::Init(&app);

        auto adsMgr = getAdsManager();

        std::array<SDL_Event, 30> eventList;
        auto& event = eventList.front();

        while (SDL_WaitEvent(&event)) { // ждем пока появится хотя бы одно событие, не выдирая его из очереди (так как параметр пустой)
            do {
                switch (event.type)
                {
                case SDL_MULTIGESTURE:
                {
                    const auto eventCount = SDL_PeepEvents(&eventList.front(), eventList.size(), SDL_GETEVENT, SDL_MULTIGESTURE, SDL_MULTIGESTURE);
                    if (eventCount > 0) { // may be negative
                        adsMgr->tryLoadInterstitial();
                    }
                }
                break;

                case SDL_MOUSEWHEEL:
                    adsMgr->showInterstitialIfAvailable();
                    break;

                default:
                    if (msgBoxEventCode == event.type) { // MessageBoxEvent
                        int Code;
                        msgBox::Action Action;
                        msgBox::DecodeEvent(event, Code, Action); // декодируем event
                        adsMgr->showInterstitialIfAvailable();
                    }
                    else if (billingEventCode == event.type) { // BillingEvent
                        int Code, Result;
                        BillingManager::DecodeEvent(event, Code, Result); // декодируем event
                        adsMgr->tryLoadInterstitial();
                    }

                    break;
                }
            } while (SDL_PollEvent(&event));
        }
    }
    return 0;
}