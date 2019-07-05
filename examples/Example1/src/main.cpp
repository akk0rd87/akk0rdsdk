#include "basewrapper.h"
#include "adrandomizer.h"
#include "atlasmanager.h"

AkkordPoint Position;
AkkordRect ZombiePosition;

AtlasManager AtlasMgr;
AdRandomizer adRand;

AkkordTexture Background;

AkkordRect rect;
AkkordTexture Ground;

AtlasManager::IndexType ZombieIndex = 0;
std::vector<AtlasManager::IndexType> ZombieVector;

auto Flip = AkkordTexture::Flip::None;

void UpdateScreen()
{
    ++ZombieIndex;
    if (ZombieIndex == ZombieVector.size()) ZombieIndex = 0;

    if (Flip == AkkordTexture::Flip::None && ZombiePosition.x > BWrapper::GetScreenWidth() - ZombiePosition.w)
        Flip = AkkordTexture::Flip::Horizontal;

    if (Flip == AkkordTexture::Flip::Horizontal && ZombiePosition.x < 0)
        Flip = AkkordTexture::Flip::None;

    int dx = ZombiePosition.w / 20;

    if (Flip == AkkordTexture::Flip::Horizontal)
        dx = -dx;

    ZombiePosition.x += dx;
}

bool Init()
{
    if (!BWrapper::Init(SDL_INIT_VIDEO)) return false;

	auto Window = BWrapper::CreateRenderWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | (BWrapper::GetDeviceOS() == BWrapper::OS::Windows ? 0 : SDL_WINDOW_BORDERLESS));

    if (!Window) return false;

    BWrapper::SetActiveWindow(Window);
    BWrapper::SetWindowResizable(true);
    auto Renderer = BWrapper::CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!Renderer)return false;

    BWrapper::SetActiveRenderer(Renderer);

    return true;
}

bool UpdateOnClick(int X, int Y)
{
    if (X < BWrapper::GetScreenWidth() / 2) Flip = AkkordTexture::Flip::Horizontal;
    else                                    Flip = AkkordTexture::Flip::None;

    return false;
}

void ReDraw()
{
    auto ScreenHeight = BWrapper::GetScreenHeight();
    auto ScreenWidth  = BWrapper::GetScreenWidth();
    BWrapper::SetCurrentColor(AkkordColor(255, 255, 255));

    BWrapper::ClearRenderer();

    Background.Draw(AkkordRect(0, 0, ScreenWidth, ScreenHeight));

    auto x   = (ScreenWidth - 400 - 10) / 3;
    auto psx = ScreenWidth / 2;

    ZombiePosition.w = 430;
    ZombiePosition.h = 519;

    if (ZombiePosition.w > BWrapper::GetScreenWidth())
    {
        ZombiePosition.w = BWrapper::GetScreenWidth();
        ZombiePosition.h = ZombiePosition.w * 519 / 430;
    }
    if (ZombiePosition.h > BWrapper::GetScreenHeight())
    {
        ZombiePosition.h = BWrapper::GetScreenHeight();
        ZombiePosition.w = ZombiePosition.h * 430 / 519;
    }

    ZombiePosition.y = (ScreenHeight - ZombiePosition.h) / 2;

    int dx = 0;
    while (dx <= ScreenWidth)
    {
        Ground.Draw(AkkordRect(dx, ZombiePosition.y + (99 * ZombiePosition.h / 100), 128, 128));
        dx += 128;
    }

    AtlasMgr.DrawSprite(ZombieVector[ZombieIndex], ZombiePosition, Flip);    

    BWrapper::RefreshRenderer();
}

void ClearAll()
{
    BWrapper::DestroyRenderer();
    BWrapper::DestroyWindow();
}


void LoadSpriteSheet()
{
    auto AtlasIndex = AtlasMgr.LoadAtlas("images/sprites.txt", "images/spritesheet.png", AtlasManager::AtlasType::LeshyLabsText);    
}


void LoadZombies()
{
    auto ZombieAtlasMale = AtlasMgr.LoadAtlas("zombiemale/sprites.txt", "zombiemale/spritesheet.png", AtlasManager::AtlasType::LeshyLabsText);

    std::string str;
    for (int i = 0; i < 10; i++)
    {
        str = "Walk (" + std::to_string(i + 1) + ")";
        auto idx = AtlasMgr.GetIndexBySpriteName(ZombieAtlasMale, str.c_str());
        ZombieVector.push_back(idx);
    }
}

int main(int, char**){

    BWrapper::SetLogPriority(BWrapper::LogPriority::Debug);
    auto LogParams = BWrapper::GetLogParams();
    LogParams->lenFile = 20;
    logDebug("Program started");
    SDL_Event event;
    if (!Init())
    {
        ClearAll();
        return 1;
    }

    logDebug(BWrapper::GetSDKVersionInfo().c_str());

	SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    switch (BWrapper::GetDeviceLanguage())
    {
        case Locale::Lang::Russian:
            logDebug("Russian;");
            break;

        case Locale::Lang::English:
            logDebug("English;");
            break;

        default:
            logDebug("Other language");
            break;
    }


    LoadZombies();
    LoadSpriteSheet();

    ZombiePosition.x = BWrapper::GetScreenWidth() / 2;

    Background.LoadFromFile("background/bg.png", AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::Assets);
    Ground.LoadFromFile("background/Tile (2).png", AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::Assets);

    ReDraw();

    auto ticks1 = BWrapper::GetTicks();
    while (1)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    goto end;
                    break;

                case SDL_WINDOWEVENT:
                {
                    switch (event.window.event) 
                    {
                        case SDL_WINDOWEVENT_FOCUS_LOST:
                            logDebug("SDL_WINDOWEVENT_FOCUS_LOST event");
                            break;

                        case SDL_WINDOWEVENT_MINIMIZED:
                            logDebug("SDL_WINDOWEVENT_MINIMIZED event");
                            break;
                    }
                    break;
                }

                case SDL_APP_WILLENTERBACKGROUND:
                {
                    logDebug("SDL_APP_WILLENTERBACKGROUND event");
                    break;
                }

                case SDL_APP_DIDENTERBACKGROUND:
                {
                    logDebug("SDL_APP_DIDENTERBACKGROUND event");
                    break;
                }

                case SDL_MOUSEBUTTONDOWN:
                {
                    UpdateOnClick(event.button.x, event.button.y);
                    break;
                }

                case SDL_KEYDOWN:
                {
                    switch (BWrapper::DecodeKey(event.key.keysym))
                    {
                        case BWrapper::KeyCodes::Back:
                        case BWrapper::KeyCodes::BackSpace:
                        case BWrapper::KeyCodes::Esc:
                            goto end;
                            break;

						case BWrapper::KeyCodes::Left:
							Flip = AkkordTexture::Flip::Horizontal;
							break;

						case BWrapper::KeyCodes::Right:
							Flip = AkkordTexture::Flip::None;
							break;
                    }
                }
                default:
                    break;
            }
        }

        auto ticks2 = SDL_GetTicks();
        if (ticks2 - ticks1 > 60)
        {
            ticks1 = ticks2;
            UpdateScreen();
            ReDraw();
        }
        BWrapper::Sleep(30);
    }

end:
    ClearAll();
    BWrapper::Quit();
    return 0;
}
