#include "basewrapper.h"
#include "billing.h"
#include "admob.h"

static int SDLCALL ShowMBox(void* n)
{
    BWrapper::OpenURL("www.sql.ru");
    return 0;
};

bool Init()
{
    int n = 0;
    if (!BWrapper::Init(SDL_INIT_VIDEO)) n = 0;//return false;

    auto Window = BWrapper::CreateRenderWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_RESIZABLE);
    
    if (!Window) n = 0;//return false;
    
    BWrapper::SetActiveWindow(Window);
    BWrapper::SetWindowResizable(true);
    auto Renderer = BWrapper::CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!Renderer) n = 0;//return false;
    
    BWrapper::SetActiveRenderer(Renderer);                

    return true;
}

bool UpdateOnClick(int X, int Y)
{   
    return false;
}

void ReDraw()
{   
    auto ScreenHeight = BWrapper::GetScreenHeight();
    auto ScreenWidth  = BWrapper::GetScreenWidth();
    BWrapper::SetCurrentColor(AkkordColor(255, 255, 255));
    
    BWrapper::ClearRenderer();    
    BWrapper::RefreshRenderer();
}

void ClearAll()
{    
    BWrapper::DestroyRenderer();
    BWrapper::DestroyWindow();
}

bool CheckSDKUvent(SDL_Event& Event)
{
    logDebug("USER EVENT");
    auto type = (CustomEvents::SDKEventType)(Event.user.code);
    switch (type)
    {
        case CustomEvents::SDKEventType::Test:
            {
                TestEvent* ev = (TestEvent*)(Event.user.data1);
                logDebug("Test event with message %s", ev->Name.c_str());
                delete ev;
            }

            default:
                break;
    }   

    return false;
}

int main(int, char**)
{       
    bool NeedRedraw = false;
    BWrapper::SetLogPriority(BWrapper::LogPriority::Debug);
    auto LogParams = BWrapper::GetLogParams();
    LogParams->lenFile = 30;
    logDebug("Program started");
    SDL_Event event;    
    if (!Init())
    {
        ClearAll();        
        return 1;
    }  

    bool ShowInterstitial = false;
    bool ShowRewardedVideo = false;

    bool ProdsQueried = false;

    //AdMob::Init("ca-app-pub-3940256099942544~3347511713", AdMob::Format::Interstitial | AdMob::Format::RewardedVideo);
    //AdMob::InterstitialSetUnitId("ca-app-pub-3940256099942544/1033173712"); // test unit id
    //AdMob::InterstitialLoad();

    //AdMob::RewardedVideoSetUnitId("ca-app-pub-3940256099942544/5224354917");
    //AdMob::RewardedVideoLoad();

    BillingManager::Init();


    ReDraw();

    

    while (SDL_WaitEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                goto end;
                break;

            case SDL_WINDOWEVENT:
                NeedRedraw = true;
                break;            
                
            case SDL_MOUSEBUTTONUP:
                //if (BWrapper::GetDeviceOS() == BWrapper::OS::AndroidOS)
                  //  BWrapper::OpenURL("www.sql.ru");
                break;

            case SDL_MOUSEBUTTONDOWN:
                UpdateOnClick(event.button.x, event.button.y);                    
                break;            

            case SDL_KEYDOWN:            
                switch (BWrapper::DecodeKey(event.key.keysym))
                {
                    case BWrapper::KeyCodes::Back:
                    case BWrapper::KeyCodes::BackSpace:
                    case BWrapper::KeyCodes::Esc:
                        BWrapper::Quit();
                        goto end;
                        break;
                }
                break;
                
            case SDL_USEREVENT:
                switch (CustomEvents::GetEventLevel(event))
                {
                    case CustomEvents::EventLevel::SDK:
                        if (CheckSDKUvent(event))
                            NeedRedraw = true;
                        break;
                }
                
                break;
                
            default:                
                break;
        }
        if (NeedRedraw)
        {
            NeedRedraw = false;
            ReDraw();
        }        
        
        if (ShowInterstitial)
        {
            auto status = AdMob::InterstitialGetStatus();
            switch (status)
            {
                case AdMob::InterstitialStatus::Inited:
                    logDebug("InterstitialStatus = Inited, Try to load");
                    AdMob::InterstitialLoad();
                    break;

                case AdMob::InterstitialStatus::Loaded:
                    logDebug("InterstitialStatus = Loaded, Try to show");
                    AdMob::InterstitialShow();
                    break;

                case AdMob::InterstitialStatus::TryingToLoad:
                    //logDebug("InterstitialStatus = TryingToLoad, waiting");
                    break;

                case AdMob::InterstitialStatus::Opened:
                    logDebug("InterstitialStatus = Opened");
                    break;

                case AdMob::InterstitialStatus::NotInited:
                    logDebug("InterstitialStatus = NotInited");
                    break;

                default:
                    logDebug("Other InterstitialStatus %d", status);
                    break;
            }
        }

        if (ShowRewardedVideo)
        {
            auto status = AdMob::RewardedVideoGetStatus();
            switch (status)
            {
                case AdMob::RewardedVideoStatus::NotInited:
                    logDebug("RewardedVideoStatus = NotInited");
                    break;
                case AdMob::RewardedVideoStatus::Inited:
                    logDebug("RewardedVideoStatus = Inited, try load");
                    AdMob::RewardedVideoLoad();
                    break;
                case AdMob::RewardedVideoStatus::TryingToLoad:
                    //logDebug("RewardedVideoStatus = TryingToLoad");
                    break;
                case AdMob::RewardedVideoStatus::Loaded:
                    logDebug("RewardedVideoStatus = Loaded, try to show");
                    AdMob::RewardedVideoShow();
                    break;
                case AdMob::RewardedVideoStatus::Opened:
                    logDebug("RewardedVideoStatus = Opened");
                    break;
                case AdMob::RewardedVideoStatus::Started:
                    logDebug("RewardedVideoStatus = Started");
                    break;
                default:
                    logDebug("Other RewardedVideoStatus %d", status);
                    break;
            }
        }       

        if (!ProdsQueried)
            if (BillingManager::GetStatus() == 0)
            {
                ProdsQueried = true;

                std::vector<std::string> ProdsList;
                ProdsList.push_back("prod_1");
                ProdsList.push_back("prod_2");
                ProdsList.push_back("prod_3");               
                
				BillingManager::QueryProductDetails(ProdsList);

            }
    }

end:
    ClearAll();
    BWrapper::Quit();    
    return 0;
}
