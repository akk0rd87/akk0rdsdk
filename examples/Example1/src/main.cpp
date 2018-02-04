#include "basewrapper.h"
#include "adrandomizer.h"
#include "atlasmanager.h"
#include "configmanager.h"
#include "admob.h"
#include <future>

AkkordPoint Position;
AkkordRect ZombiePosition;

AtlasManager AtlasMgr;
AdRandomizer adRand;
AkkordRect AdImagesRects[4];

AkkordTexture Background;

AkkordRect rect;
AkkordTexture Ground;

unsigned ZombieIndex = 0;
std::vector<unsigned> ZombieVector;

unsigned ImageFacebookIndex;
unsigned ImageeraserIndex;
unsigned ImageBackIndex;

auto Flip = AkkordTexture::Flip::None;

void UpdateScreen()
{
    ++ZombieIndex;
    //logDebug("ZombieIndex %u", ZombieIndex);
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
    
    for (unsigned i = 0; i < 4; i++)
    {         
        if (BWrapper::IsPointInRect(AkkordPoint(X, Y), AdImagesRects[i]))
        {
            adRand.OpenURLByIndex(i);
            logDebug("Clicked Ad Image = %d", i);
        }
    }
    

    if (X < BWrapper::GetScreenWidth() / 2) Flip = AkkordTexture::Flip::Horizontal;
    else                                    Flip = AkkordTexture::Flip::None;

    /*
    auto length = BWrapper::AndroidToastDuration::Long;
    std::string Message;
    if (Y < BWrapper::GetScreenHeight() / 2)
    {
        length  = BWrapper::AndroidToastDuration::Short;
        Message = "Click at (" + std::to_string(X) + ", " + std::to_string(Y) + ") Short";
    }
    else
    {
        Message = "Click at (" + std::to_string(X) + ", " + std::to_string(Y) + ") Long";
    }

    BWrapper::AndroidShowToast(Message.c_str(), length);
    */

    return false;
}

void ReDraw()
{    
    //logDebug("Redraw() ZombieIndex = %u", ZombieIndex);
    auto ScreenHeight = BWrapper::GetScreenHeight();
    auto ScreenWidth  = BWrapper::GetScreenWidth();
    BWrapper::SetCurrentColor(AkkordColor(255, 255, 255));
    
    BWrapper::ClearRenderer();    
    
    Background.Draw(AkkordRect(0, 0, ScreenWidth, ScreenHeight));

    
    AdImagesRects[0].x = 10;
    AdImagesRects[0].y = ScreenHeight - AdImagesRects[0].h - 10;

    AdImagesRects[1].x = ScreenWidth - AdImagesRects[1].w - 10;
    AdImagesRects[1].y = ScreenHeight - AdImagesRects[1].h - 10;

    unsigned x   = (ScreenWidth - 400 - 10) / 3;
    unsigned psx = ScreenWidth / 2;

    AdImagesRects[2].x = psx - AdImagesRects[2].w - x / 2;
    AdImagesRects[2].y = ScreenHeight - AdImagesRects[0].h - 10;

    AdImagesRects[3].x = psx + x / 2;
    AdImagesRects[3].y = ScreenHeight - AdImagesRects[0].h - 10;

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

    //rect.x = (ScreenWidth - rect.w) / 2;
    ZombiePosition.y = (ScreenHeight - ZombiePosition.h) / 2;

    int dx = 0;
    while (dx <= ScreenWidth)
    {
        Ground.Draw(AkkordRect(dx, ZombiePosition.y + (99 * ZombiePosition.h / 100), 128, 128));
        dx += 128;
    }

    AtlasMgr.DrawSprite(ZombieVector[ZombieIndex], ZombiePosition, Flip);
    
    for (unsigned i = 0; i < 4; i++)
        adRand.DrawImageByIndex(i, AdImagesRects[i]);

    AtlasMgr.DrawSprite(ImageFacebookIndex, AkkordRect(0, 0, 100, 100));

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
    ImageFacebookIndex = AtlasMgr.GetIndexBySpriteName(AtlasIndex, "facebook");
    ImageeraserIndex = AtlasMgr.GetIndexBySpriteName(AtlasIndex, "eraser");
    ImageBackIndex = AtlasMgr.GetIndexBySpriteName(AtlasIndex, "back");
}


void LoadZombies()
{
    auto ZombieAtlasMale = AtlasMgr.LoadAtlas("zombiemale/sprites.txt", "zombiemale/spritesheet.png", AtlasManager::AtlasType::LeshyLabsText);
    //auto ZombieAtlasFeMale = AtlasManager::LoadAtlas("zombiemale/sprites.txt", "zombiemale/spritesheet.png", AtlasManager::AtlasType::LeshyLabsText);

    std::string str;
    for (int i = 0; i < 10; i++)
    {
        str = "Walk (" + std::to_string(i + 1) + ")";
        auto idx = AtlasMgr.GetIndexBySpriteName(ZombieAtlasMale, str.c_str());
        ZombieVector.push_back(idx);
    }

    //logDebug("ZombieVector.size = %u", ZombieVector.size());
}

/*
void Loop()
{
    unsigned n = 10;
    while (n)
    {
        logDebug("Thread");
        BWrapper::Sleep(1000);
        --n;
    }
    logDebug("Thread Finished");
}
*/

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
    
    //logDebug("Android api level %d", BWrapper::AndroidGetApiLevel());

    // Add Rects to Control Clicks by Images
    int imagesCnt = adRand.Randomize(4);
    for (int i = 0; i < imagesCnt; i++)
    {    
        AdImagesRects[i].w = adRand.ImageWidth;
        AdImagesRects[i].h = adRand.ImageHeight;
    } 
                
    LoadZombies();
    LoadSpriteSheet();    

    ZombiePosition.x = BWrapper::GetScreenWidth() / 2;

    Background.LoadFromFile("background/bg.png", AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::Assets);
    Ground.LoadFromFile("background/Tile (2).png", AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::Assets);
    
    ReDraw();

    //auto handle = std::async(std::launch::async, Loop);

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
                    break;
                }

                case SDL_MOUSEBUTTONDOWN:
                {
                    UpdateOnClick(event.button.x, event.button.y);
                    //BWrapper::OpenURL("www.sql.ru");
                    break;
                }

                case SDL_KEYDOWN:
                {
                    switch (BWrapper::DecodeKey(event.key.keysym))
                    {
                        case BWrapper::KeyCodes::Back:
                        case BWrapper::KeyCodes::BackSpace:
                        case BWrapper::KeyCodes::Esc:
                            BWrapper::Quit();
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
			auto Size = BWrapper::GetScreenSize();
			logDebug("Ticks %u, w = %d, h = %d", ticks2, Size.x, Size.y);
            ticks1 = ticks2;
            UpdateScreen();
            ReDraw();
        }
        SDL_Delay(30);
    }

end:
    ClearAll();
    BWrapper::Quit();    
    return 0;
}
