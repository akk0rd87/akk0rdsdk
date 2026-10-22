// до basewrapper.h: реализация SDL_gesture.h находится внутри его include guard
#define SDL_GESTURE_IMPLEMENTATION
#include "basewrapper.h"
#include "core/platformwrapper/platforms.h"
#include "core/core_defines.h"
#include <ctime>

static_assert (!std::numeric_limits<char>::is_signed, "Char data type must be as unsigned char data type. Set compiler flag -funsigned-char for GCC or /J for MSVC");

#define NANOSVG_IMPLEMENTATION
#include "../libraries/nanosvg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "../libraries/nanosvg/nanosvgrast.h"

class CurrentContextStruct
{
public:
    AkkordRenderer* CurrentRenderer = nullptr;
    AkkordWindow* CurrentWindow = nullptr;
    Uint32                    MessageBoxEvent;

    void DestroyRenderer()
    {
        if (this->CurrentRenderer != nullptr)
        {
            SDL_DestroyRenderer(this->CurrentRenderer);
            this->CurrentRenderer = nullptr;
        }
    };

    void DestroyWindow()
    {
        if (this->CurrentWindow != nullptr)
        {
            SDL_DestroyWindow(this->CurrentWindow);
            this->CurrentWindow = nullptr;
        }
    };

    ~CurrentContextStruct()
    {
        DestroyRenderer();
        DestroyWindow();
    }
};

static CurrentContextStruct CurrentContext;
static LogParamsStruct LogParams;

bool BWrapper::Init(Uint32 flags)
{
    //Если раскомментить нижеприведенную строку, то на винде валится при закрытии окна приложения (не консоли)
    //if (SDL_SetMemoryFunctions(std::malloc, std::calloc, std::realloc, std::free) != 0)
      //  logError("SDL_SetMemoryFunctions error %s", SDL_GetError());

    if (!SDL_Init(flags))
    {
        logError("BWrapper::Init: Error %s", SDL_GetError());
        return false;
    }

    if (Gesture_Init() != 0) {
        logError("BWrapper::Init: Gesture_Init error %s", SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);

    CurrentContext.MessageBoxEvent = SDL_RegisterEvents(1);
    PlatformWrapper::GetInstance().Init();
    return true;
};

////////////////////////////////
//////// FILE FUNCTIONS
////////////////////////////////

FILE* FileOpen_private(const char* Filename, BWrapper::FileSearchPriority SearchPriority, BWrapper::FileOpenMode OpenMode)
{
    // Здесь не должно быть никакого ERROR-логирования, так как эта процедура будет использоваться для IfFileExists
    logVerbose("FileOpen_private = %s", Filename);

    // Assets accesable only in read mode
    if (BWrapper::FileSearchPriority::Assets == SearchPriority)
    {
        if (BWrapper::FileOpenMode::WriteBinary == OpenMode || BWrapper::FileOpenMode::AppendBinary == OpenMode /*|| BWrapper::FileOpenMode::WriteText == OpenMode*/)
        {
            logError("FileOpen_private: Opening assets %s with write mode is not allowed", Filename);
            return nullptr;
        }
    }

    const char* mode = nullptr;

    switch (OpenMode)
    {
    case BWrapper::FileOpenMode::ReadBinary:
        mode = "rb";
        break;
    case BWrapper::FileOpenMode::WriteBinary:
        mode = "wb";
        break;
    case BWrapper::FileOpenMode::AppendBinary:
        mode = "ab";
        break;
        /*
    case BWrapper::FileOpenMode::ReadText:
        strcpy(mode, "r");
        break;
    case BWrapper::FileOpenMode::WriteText:
        strcpy(mode, "w");
        break;
        */
    default:
        logError("Fopen_private %s: FileOpenMode is not supported", Filename);
        return nullptr;
        break;
    }

    return fopen(Filename, mode);
}

FILE* BWrapper::FileOpen(const char* FileName, FileSearchPriority SearchPriority, FileOpenMode OpenMode)
{
    // Assets not accesable by this method yet
    if (FileSearchPriority::Assets == SearchPriority)
    {
        logError("BWrapper::FileOpen: Opening asset %s is not allowed", FileName);
        return nullptr;
    }

    auto File = FileOpen_private(FileName, SearchPriority, OpenMode);

    if (nullptr == File)
        logError("BWrapper::FileOpen: File %s open error", FileName);

    return File;
}

bool BWrapper::FileExists(const char* FileName, BWrapper::FileSearchPriority SearchPriority)
{
    // На андроиде assets читаются особым образом
    if (BWrapper::GetDeviceOS() == BWrapper::OS::AndroidOS && BWrapper::FileSearchPriority::Assets == SearchPriority) {
        return (PlatformWrapper::GetInstance().GetFileBuf(FileName, SearchPriority) ? true : false);
    }

    auto File = FileOpen_private(FileName, SearchPriority, BWrapper::FileOpenMode::ReadBinary);

    if (File) {
        FileClose(File);
        return true;
    }
    return false;
}

void BWrapper::FileClose(FILE*& File)
{
    if (File) {
        fclose(File);
    }
    File = nullptr;
}

bool BWrapper::FileWrite(FILE* File, const void* Buffer, size_t Size, size_t Count) // http://www.cplusplus.com/reference/cstdio/fwrite/
{
    if (fwrite(Buffer, Size, Count, File) == Count) return true;
    return false;
}

bool BWrapper::FileWriteFormatted(FILE* File, const char* Format, ...) // http://www.cplusplus.com/reference/cstdio/vfprintf/
{
    va_list args;
    va_start(args, Format);
    vfprintf(File, Format, args);
    va_end(args);

    return true;
}

bool BWrapper::FilePutS(FILE* File, const char* String) // http://www.cplusplus.com/reference/cstdio/fputs/
{
    if (fputs(String, File) >= 0) return true;
    logError("BWrapper::FilePutS Error");
    return false;
}

bool BWrapper::DirCreate(const char* Dir)
{
    return PlatformWrapper::GetInstance().DirCreate(Dir);
}

bool BWrapper::FileDelete(const char* FileName)
{
    logVerbose("BWrapper::FileDelete File %s", FileName);
    if (remove(FileName) == 0) return true;
    logError("BWrapper::FileDelete Error Delete File %s", FileName);
    return false;
}

bool BWrapper::FileRename(const char* OldName, const char* NewName)
{
    logVerbose("BWrapper::FileRename %s to %s", OldName, NewName);
    if (rename(OldName, NewName) == 0) return true;
    logError("BWrapper::FileRename Error Rename %s to %s", OldName, NewName);
    return false;
}

////////////////////////////////
/////////
////////////////////////////////

AkkordWindow* BWrapper::CreateRenderWindow(const char* Title, int X, int Y, int W, int H, SDL_WindowFlags Flags)
{
    auto wnd = SDL_CreateWindow(Title, W, H, Flags);
    if (nullptr == wnd) {
        logError("CreateWindow error = %s", SDL_GetError());
        return nullptr;
    }
    // в SDL3 позиция задается отдельно; SDL_WINDOWPOS_UNDEFINED оставляет выбор за системой
    if (!SDL_WINDOWPOS_ISUNDEFINED(X) || !SDL_WINDOWPOS_ISUNDEFINED(Y)) {
        SDL_SetWindowPosition(wnd, X, Y);
    }
    return wnd;
};

AkkordRenderer* BWrapper::CreateRenderer(AkkordWindow* window, const char* DriverName)
{
    auto rnd = SDL_CreateRenderer(window, DriverName);
    if (nullptr == rnd)
        logError("CreateRenderer error = %s", SDL_GetError());
    else
        logDebug("Renderer: %s", SDL_GetRendererName(rnd));
    return rnd;
};

bool BWrapper::SetActiveWindow(AkkordWindow* Window)
{
    CurrentContext.CurrentWindow = Window;
    return true;
};

bool BWrapper::SetActiveRenderer(AkkordRenderer* Renderer)
{
    CurrentContext.CurrentRenderer = Renderer;
    return true;
};

AkkordWindow* BWrapper::GetActiveWindow()
{
    return CurrentContext.CurrentWindow;
};

AkkordRenderer* BWrapper::GetActiveRenderer()
{
    return CurrentContext.CurrentRenderer;
};

bool BWrapper::ClearRenderer()
{
    SDL_RenderClear(CurrentContext.CurrentRenderer);
    return true;
};

bool BWrapper::SetWindowResizable(bool Resizable)
{
    SDL_SetWindowResizable(CurrentContext.CurrentWindow, true == Resizable ? true : false);
    return true;
}

bool BWrapper::SetWindowFullscreen(bool Fullscreen)
{
    // в SDL3 без SDL_SetWindowFullscreenMode окно уходит в desktop fullscreen (аналог SDL_WINDOW_FULLSCREEN_DESKTOP)
    if (!SDL_SetWindowFullscreen(CurrentContext.CurrentWindow, Fullscreen)) {
        logError("SetWindowFullscreen error = %s", SDL_GetError());
        return false;
    }
    PlatformWrapper::GetInstance().OnWindowFullscreenChanged(Fullscreen);
    return true;
}

bool BWrapper::IsWindowFullscreen()
{
    return PlatformWrapper::GetInstance().IsWindowFullscreen(CurrentContext.CurrentWindow);
}

bool BWrapper::DestroyRenderer()
{
    CurrentContext.DestroyRenderer();
    return true;
};

bool BWrapper::RefreshRenderer()
{
    SDL_RenderPresent(CurrentContext.CurrentRenderer);
    return true;
};

bool BWrapper::FlushRenderer() {
    SDL_FlushRenderer(CurrentContext.CurrentRenderer);
    return true;
};

bool BWrapper::DestroyWindow()
{
    CurrentContext.DestroyWindow();
    return true;
};

bool BWrapper::SetWindowSize(int W, int H)
{
    SDL_SetWindowSize(CurrentContext.CurrentWindow, W, H);
    return true;
}
bool AkkordTexture::CreateFromSurface(SDL_Surface* Surface)
{
    tex = std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)>(SDL_CreateTextureFromSurface(CurrentContext.CurrentRenderer, Surface), SDL_DestroyTexture);
    if (!tex) {
        logError("Error create texture from surface %s", SDL_GetError());
        return false;
    }
    return true;
};

bool AkkordTexture::LoadFromMemory(const char* Buffer, int Size, TextureType Type, float Scale)
{
    if (nullptr == Buffer) {
        logError("Error load texture from memory: buffer is not specified");
        return false;
    }

    std::unique_ptr<SDL_IOStream, void(*)(SDL_IOStream*)>io(SDL_IOFromMem((void*)Buffer, Size), [](SDL_IOStream* i) {SDL_CloseIO(i); });
    bool result = false;

    if (io) {
        auto image = [](decltype(io)& IO, AkkordTexture::TextureType TextureType, float Scale) {
            switch (TextureType)
            {
            // IO закрывает unique_ptr, поэтому closeio = false
            case AkkordTexture::TextureType::BMP:
                return std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>(SDL_LoadBMP_IO(IO.get(), false), SDL_DestroySurface);
                break;

            case AkkordTexture::TextureType::PNG:
                return std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>(SDL_LoadPNG_IO(IO.get(), false), SDL_DestroySurface);
                break;

            case AkkordTexture::TextureType::JPEG:
                // SDL3 без SDL_image не грузит JPEG
                logError("JPEG images are not supported");
                break;

            case AkkordTexture::TextureType::SVG:
            {
                std::unique_ptr<char, void(*)(char*)> data((char*)SDL_LoadFile_IO(IO.get(), nullptr, false), [](char* i) { SDL_free(i); });
                if (data) {
                    std::unique_ptr<NSVGimage, void(*)(NSVGimage*)> svg_image(nsvgParse(data.get(), "px", 96.0f), nsvgDelete);
                    if (svg_image) {
                        std::unique_ptr<NSVGrasterizer, void(*)(NSVGrasterizer*)>rasterizer(nsvgCreateRasterizer(), nsvgDeleteRasterizer);
                        if (rasterizer) {
                            // nanosvg пишет байты в порядке R, G, B, A
                            std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)> image(
                                SDL_CreateSurface(
                                    static_cast<int>(svg_image->width * Scale),
                                    static_cast<int>(svg_image->height * Scale),
                                    SDL_PIXELFORMAT_RGBA32),
                                SDL_DestroySurface
                            );
                            if (image) {
                                nsvgRasterize(rasterizer.get(), svg_image.get(), 0.0f, 0.0f, Scale, static_cast<unsigned char*>(image->pixels), image->w, image->h, image->pitch);
                                return image;
                            }
                            else {
                                logError("Couldn't create SDL_CreateSurface %s", SDL_GetError());
                            }
                        }
                        else {
                            logError("Couldn't create SVG rasterizer %s", SDL_GetError());
                        }
                    }
                    else {
                        logError("Couldn't parse SVG image %s", SDL_GetError());
                    }
                }
                else {
                    logError("Couldn't parse SVG image %s", SDL_GetError());
                }
            }
            break;

            default:
                logError("Invalid texture type");
                break;
            }
            return std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>(nullptr, SDL_DestroySurface);
        }(io, Type, Scale);

        if (image) {
            result = this->CreateFromSurface(image.get());
        }
        else {
            logError("Error load Image SDL_IOFromMem error=%s", SDL_GetError());
        }
    }

    return result;
};

bool AkkordTexture::LoadFromFile(const char* FileName, TextureType Type, const BWrapper::FileSearchPriority SearchPriority, float Scale)
{
    if (tex) {
        this->Destroy();
    }

    auto fb = PlatformWrapper::GetInstance().GetFileBuf(FileName, SearchPriority);

    if (!fb) {
        logError("Error load file image = %s", FileName);
        return false;
    }

    if (!this->LoadFromMemory(fb->Begin(), fb->End() - fb->Begin(), Type, Scale)) {
        logError("Error load file %s", FileName);
        return false;
    }

    return true;
};

bool AkkordTexture::SetColorMod(Uint8 R, Uint8 G, Uint8 B)
{
    if (SDL_SetTextureColorMod(tex.get(), R, G, B))
        return true;

    logError("SDL_SetTextureColorMod error: %s", SDL_GetError());
    return false;
}

bool AkkordTexture::SetAlphaMod(Uint8 A)
{
    if (SDL_SetTextureAlphaMod(tex.get(), A))
        return true;

    logError("SDL_SetTextureAlphaMod error: %s", SDL_GetError());
    return false;
};

bool AkkordTexture::Draw(const AkkordRect& Rect, const AkkordRect* RectFromAtlas) const
{
    // в SDL3 функции рисования принимают только float-прямоугольники
    SDL_FRect tgtRect, srcRect;
    SDL_RectToFRect(&Rect, &tgtRect);
    if (RectFromAtlas) {
        SDL_RectToFRect(RectFromAtlas, &srcRect);
    }

    if (!SDL_RenderTexture(CurrentContext.CurrentRenderer, tex.get(), (RectFromAtlas ? &srcRect : nullptr), &tgtRect)) {
        logError("Error draw image %s", SDL_GetError());
        return false;
    }
    return true;
};

bool AkkordTexture::Draw(const AkkordRect& Rect, const AkkordRect* RectFromAtlas, AkkordTexture::Flip Flip, double Angle, AkkordPoint* Point) const
{
    // converting Flip
    const auto flip = static_cast<SDL_FlipMode>(Flip);

    SDL_FRect tgtRect, srcRect;
    SDL_RectToFRect(&Rect, &tgtRect);
    if (RectFromAtlas) {
        SDL_RectToFRect(RectFromAtlas, &srcRect);
    }

    SDL_FPoint centerPoint;
    if (Point) {
        centerPoint.x = static_cast<float>(Point->GetX());
        centerPoint.y = static_cast<float>(Point->GetY());
    }

    if (!SDL_RenderTextureRotated(CurrentContext.CurrentRenderer, tex.get(), (RectFromAtlas ? &srcRect : nullptr), &tgtRect, Angle, (Point ? &centerPoint : nullptr), flip)) {
        logError("Error draw image %s", SDL_GetError());
        return false;
    }
    return true;
};

AkkordPoint AkkordTexture::GetSize() const
{
    AkkordPoint Point(-1, -1);
    if (tex)
    {
        Point.x = tex->w;
        Point.y = tex->h;
    }
    else
    {
        logError("AkkordTexture::GetSize():  Texture is empty");
    }
    return Point;
};

AkkordPoint BWrapper::GetScreenSize()
{
    AkkordPoint WSize;
    //SDL_GetWindowSize(CurrentContext.CurrentWindow, &WSize.x, &WSize.y);
    //SDL_GL_GetDrawableSize(CurrentContext.CurrentWindow, &WSize.x, &WSize.y);
    SDL_GetCurrentRenderOutputSize(CurrentContext.CurrentRenderer, &WSize.x, &WSize.y);
    return WSize;
};

AkkordRect BWrapper::GetWindowSafeArea() {
    return PlatformWrapper::GetInstance().GetWindowSafeArea(CurrentContext.CurrentRenderer, CurrentContext.CurrentWindow);
}

void BWrapper::SetSystemBarsAppearance(bool LightStatusBar, bool LightNavigationBar) {
    PlatformWrapper::GetInstance().SetSystemBarsAppearance(LightStatusBar, LightNavigationBar);
}

bool BWrapper::SetCurrentColor(const AkkordColor& Color)
{
    if (SDL_SetRenderDrawColor(CurrentContext.CurrentRenderer, Color.GetR(), Color.GetG(), Color.GetB(), Color.GetA())) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
}

bool BWrapper::DrawRect(const AkkordRect& Rect)
{
    SDL_FRect fRect;
    SDL_RectToFRect(&Rect, &fRect);
    if (SDL_RenderRect(CurrentContext.CurrentRenderer, &fRect)) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

bool BWrapper::DrawFRect(const AkkordFRect& Rect)
{
    if (SDL_RenderRect(CurrentContext.CurrentRenderer, &Rect)) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

bool BWrapper::FillRect(const AkkordRect& Rect)
{
    SDL_FRect fRect;
    SDL_RectToFRect(&Rect, &fRect);
    if (SDL_RenderFillRect(CurrentContext.CurrentRenderer, &fRect)) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

bool BWrapper::FillFRect(const AkkordFRect& Rect) {
    if (SDL_RenderFillRect(CurrentContext.CurrentRenderer, &Rect)) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
}

bool BWrapper::DrawRect(const AkkordFRect& Rect) {
    if (SDL_RenderRect(CurrentContext.CurrentRenderer, &Rect)) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

bool BWrapper::FillRect(const AkkordFRect& Rect) {
    if (SDL_RenderFillRect(CurrentContext.CurrentRenderer, &Rect)) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

bool BWrapper::DrawLine(const AkkordPoint& Point1, const AkkordPoint& Point2)
{
    if (SDL_RenderLine(CurrentContext.CurrentRenderer, static_cast<float>(Point1.GetX()), static_cast<float>(Point1.GetY()), static_cast<float>(Point2.GetX()), static_cast<float>(Point2.GetY()))) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

bool BWrapper::DrawFLine(const AkkordFPoint& Point1, const AkkordFPoint& Point2)
{
    if (SDL_RenderLine(CurrentContext.CurrentRenderer, Point1.GetX(), Point1.GetY(), Point2.GetX(), Point2.GetY())) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

std::string BWrapper::Int2Str(int Num)
{
    return std::to_string(Num);
}

unsigned BWrapper::Str2Num(const char* Str)
{
    unsigned num;
    for (num = 0; '0' <= *Str && *Str <= '9'; Str++)
        num = (10 * num) + (*Str - '0');

    return num;
}

BWrapper::KeyCodes BWrapper::DecodeKey(SDL_Keycode Key)
{
    //enum struct KeyCodes { Esc, BackSpace, Back, Enter, Tab, Delete, F1,
    // Help, Home, Insert, Find, Copy, PageDown, PageUp, Paste, Pause, PrintScreen, Return, Space, Uknown };

    switch (Key)
    {
    case SDLK_ESCAPE: return KeyCodes::Esc;
    case SDLK_AC_BACK: return KeyCodes::Back;
    case SDLK_BACKSPACE: return KeyCodes::BackSpace;
    case SDLK_KP_ENTER: return KeyCodes::Enter;
    case SDLK_KP_TAB: return KeyCodes::Tab;
    case SDLK_DELETE: return KeyCodes::Delete;
    case SDLK_F1: return KeyCodes::F1;
    case SDLK_HELP: return KeyCodes::Help;
    case SDLK_HOME: return KeyCodes::Home;
    case SDLK_END: return KeyCodes::End;
    case SDLK_INSERT: return KeyCodes::Insert;
    case SDLK_FIND: return KeyCodes::Find;
    case SDLK_COPY: return KeyCodes::Copy;
    case SDLK_PAGEDOWN: return KeyCodes::PageDown;
    case SDLK_PAGEUP: return KeyCodes::PageUp;
    case SDLK_PASTE: return KeyCodes::Paste;
    case SDLK_PAUSE: return KeyCodes::Pause;
    case SDLK_PRINTSCREEN: return KeyCodes::PrintScreen;
    case SDLK_RETURN: return KeyCodes::Return;
    case SDLK_RETURN2: return KeyCodes::Return2;
    case SDLK_SPACE: return KeyCodes::Space;
    case SDLK_LEFT: return KeyCodes::Left;
    case SDLK_RIGHT: return KeyCodes::Right;
    case SDLK_UP: return KeyCodes::Up;
    case SDLK_DOWN: return KeyCodes::Down;
    case SDLK_MINUS: return KeyCodes::Minus;
    case SDLK_PLUS: return KeyCodes::Plus;
    case SDLK_EQUALS: return KeyCodes::Equals;
    case SDLK_KP_MINUS: return KeyCodes::NumpadMinus;
    case SDLK_KP_PLUS: return KeyCodes::NumpadPlus;

    case SDLK_LEFTBRACKET:  return KeyCodes::LeftBraket;
    case SDLK_RIGHTBRACKET: return KeyCodes::RightBraket;
    case SDLK_COMMA:        return KeyCodes::Comma;
    case SDLK_PERIOD:       return KeyCodes::Period;
    case SDLK_APOSTROPHE:   return KeyCodes::Quote;

    case SDLK_KP_0: return KeyCodes::Numpad0;
    case SDLK_KP_1: return KeyCodes::Numpad1;
    case SDLK_KP_2: return KeyCodes::Numpad2;
    case SDLK_KP_3: return KeyCodes::Numpad3;
    case SDLK_KP_4: return KeyCodes::Numpad4;
    case SDLK_KP_5: return KeyCodes::Numpad5;
    case SDLK_KP_6: return KeyCodes::Numpad6;
    case SDLK_KP_7: return KeyCodes::Numpad7;
    case SDLK_KP_8: return KeyCodes::Numpad8;
    case SDLK_KP_9: return KeyCodes::Numpad9;

    case SDLK_0: return KeyCodes::N0;
    case SDLK_1: return KeyCodes::N1;
    case SDLK_2: return KeyCodes::N2;
    case SDLK_3: return KeyCodes::N3;
    case SDLK_4: return KeyCodes::N4;
    case SDLK_5: return KeyCodes::N5;
    case SDLK_6: return KeyCodes::N6;
    case SDLK_7: return KeyCodes::N7;
    case SDLK_8: return KeyCodes::N8;
    case SDLK_9: return KeyCodes::N9;

    case SDLK_A: return KeyCodes::A;
    case SDLK_B: return KeyCodes::B;
    case SDLK_C: return KeyCodes::C;
    case SDLK_D: return KeyCodes::D;
    case SDLK_E: return KeyCodes::E;
    case SDLK_F: return KeyCodes::F;
    case SDLK_G: return KeyCodes::G;
    case SDLK_H: return KeyCodes::H;
    case SDLK_I: return KeyCodes::I;
    case SDLK_J: return KeyCodes::J;
    case SDLK_K: return KeyCodes::K;
    case SDLK_L: return KeyCodes::L;
    case SDLK_M: return KeyCodes::M;
    case SDLK_N: return KeyCodes::N;
    case SDLK_O: return KeyCodes::O;
    case SDLK_P: return KeyCodes::P;
    case SDLK_Q: return KeyCodes::Q;
    case SDLK_R: return KeyCodes::R;
    case SDLK_S: return KeyCodes::S;
    case SDLK_T: return KeyCodes::T;
    case SDLK_U: return KeyCodes::U;
    case SDLK_V: return KeyCodes::V;
    case SDLK_W: return KeyCodes::W;
    case SDLK_X: return KeyCodes::X;
    case SDLK_Y: return KeyCodes::Y;
    case SDLK_Z: return KeyCodes::Z;

    default: return KeyCodes::Uknown;
    }
}

/*
void BWrapper::MessageBoxSetColorScheme(MessageBoxColorScheme& Scheme)
{
    MessageBoxColorScheme2Native(&Scheme, CurrentContext.MessageBoxColorScheme);
}
*/

//int BWrapper::ShowMessageBox(const char* Message)
//{
//    // https://wiki.libsdl.org/SDL_ShowMessageBox
//    // https://wiki.libsdl.org/SDL_ShowSimpleMessageBox
//    const SDL_MessageBoxButtonData buttons[] = {
//        { /* .flags, .buttonid, .text */        0, 0, "Ok" },
//        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "yes" },
//        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "cancel" },
//    };
//    const SDL_MessageBoxColorScheme colorScheme = {
//        { /* .colors (.r, .g, .b) */
//            /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
//            { 88, 135, 63 },
//            /* [SDL_MESSAGEBOX_COLOR_TEXT] */
//            { 250, 250, 250 },
//            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
//            { 255, 255, 0 },
//            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
//            { 0, 0, 255 },
//            /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
//            { 255, 0, 255 }
//        }
//    };
//
//    const SDL_MessageBoxData messageboxdata = {
//        SDL_MESSAGEBOX_INFORMATION, /* .flags */
//        //NULL, /* .window */
//        CurrentContext.CurrentWindow,
//        //NULL,
//        "example message box", /* .title */
//        Message, /* .message */
//        SDL_arraysize(buttons), /* .numbuttons */
//        buttons, /* .buttons */
//        &colorScheme /* .colorScheme */
//    };
//
//    int buttonid;
//
//    if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
//        SDL_Log("error displaying message box");
//        return 1;
//    }
//    if (buttonid == -1) {
//        SDL_Log("no selection");
//    }
//    else {
//        SDL_Log("selection was %s", buttons[buttonid].text);
//    }
//
//    return 0; // Заглушка
//}

int BWrapper::GetDisplayDPI(int DisplayIndex, float* Ddpi, float* Hdpi, float* Vdpi)
{
    // DPI при масштабе 1.0: так считают сами системы (Windows — 96, Android — mdpi 160, iOS — 1x 160)
    constexpr float baseDPI = (BWrapper::GetDeviceOS() == BWrapper::OS::Windows ? 96.0F : 160.0F);

    // в SDL3 дисплеи адресуются по SDL_DisplayID, а не по индексу
    int count = 0;
    std::unique_ptr<SDL_DisplayID, void(*)(void*)> displays(SDL_GetDisplays(&count), SDL_free);
    if (!displays || DisplayIndex < 0 || DisplayIndex >= count) {
        logError("GetDisplayDPI: display %d not found: %s", DisplayIndex, SDL_GetError());
        return -1;
    }
    const auto displayId = displays.get()[DisplayIndex];

    // у окна на этом дисплее берем полный масштаб (плотность пикселей * масштаб контента): на iOS масштаб контента всегда 1.0,
    // а Retina-множитель — это плотность пикселей окна
    float scale = 0.0F;
    if (CurrentContext.CurrentWindow && SDL_GetDisplayForWindow(CurrentContext.CurrentWindow) == displayId) {
        scale = SDL_GetWindowDisplayScale(CurrentContext.CurrentWindow);
    }
    if (scale <= 0.0F) {
        scale = SDL_GetDisplayContentScale(displayId);
    }
    if (scale <= 0.0F) {
        logError("GetDisplayDPI: display scale error: %s", SDL_GetError());
        return -1;
    }

    const float dpi = baseDPI * scale;
    if (Ddpi) *Ddpi = dpi;
    if (Hdpi) *Hdpi = dpi;
    if (Vdpi) *Vdpi = dpi;
    return 0;
}

void BWrapper::Quit()
{
    Gesture_Quit();
    SDL_Quit();
    //atexit(SDL_Quit);
}

bool BWrapper::OpenURL(const char* url)
{
    if (PlatformWrapper::GetInstance().OpenURL(url))
        return true;

    logError("URL %s was not opened", url);
    return false;
};

std::string BWrapper::GetAppVersionCode() {
    return PlatformWrapper::GetInstance().GetAppVersionCode();
};

std::string BWrapper::GetAppVersionName() {
    return PlatformWrapper::GetInstance().GetAppVersionName();
};

Locale::Lang BWrapper::GetDeviceLanguage()
{
    return PlatformWrapper::GetInstance().GetDeviceLanguage();
    //return Locale::Lang::Unknown;
};

int BWrapper::AndroidGetApiLevel()
{
    return PlatformSpecific::AndroidGetApiLevel();
}

bool BWrapper::AndroidShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset)
{
    return PlatformSpecific::AndroidShowToast(Message, Duration, Gravity, xOffset, yOffset);
}

LogParamsStruct* BWrapper::GetLogParams()
{
    return &LogParams;
}

void BWrapper::Log(BWrapper::LogPriority Priority, const char* File, const char* Function, unsigned Line, SDL_PRINTF_FORMAT_STRING const char* Fmt, ...)
{
#ifdef __AKK0RD_SDK_DEBUG_MACRO__
    // https://wiki.libsdl.org/CategoryLog
    SDL_LogPriority sev = (SDL_LogPriority)Priority;

    int len = 0;
    switch (Priority)
    {
    case BWrapper::LogPriority::Verbose: len = 7; break;
    case BWrapper::LogPriority::Debug: len = 5; break;
    case BWrapper::LogPriority::Info: len = 4; break;
    case BWrapper::LogPriority::Warning: len = 4; break; /* base WARN*/
    case BWrapper::LogPriority::Error: len = 5; break;
    case BWrapper::LogPriority::Critical: len = 8; break;
    }
    std::string Format(8 - len, ' ');

    {
        auto ms = BWrapper::GetTicks();
        const auto hh24 = ms / 60 / 60 / 1000;
        ms = ms - hh24 * 60 / 60 / 1000;
        const auto mi = ms / 60 / 1000;
        ms = ms - mi * 60 * 1000;
        const auto ss = ms / 1000;
        ms = ms % 1000;

        char TimeBuffer[16];
        TimeBuffer[0] = hh24 / 10 + '0';
        TimeBuffer[1] = hh24 % 10 + '0';
        TimeBuffer[2] = ':';
        TimeBuffer[3] = mi / 10 + '0';
        TimeBuffer[4] = mi % 10 + '0';
        TimeBuffer[5] = ':';
        TimeBuffer[6] = ss / 10 + '0';
        TimeBuffer[7] = ss % 10 + '0';
        TimeBuffer[8] = '.';
        TimeBuffer[9] = ms / 100 + '0';
        TimeBuffer[10] = ms % 100 / 10 + '0';
        TimeBuffer[11] = ms % 10 + '0';
        TimeBuffer[12] = ' ';
        TimeBuffer[13] = '|';
        TimeBuffer[14] = 32;
        TimeBuffer[15] = 0;
        Format += TimeBuffer;
    }
    std::string sLine;

    // Add File Info
    if (LogParams.showFile) {
        auto pos = std::string(File).find_last_of("\\/");
        if (pos != std::string::npos) {
            auto& sFile = sLine;
            sFile.assign(File, pos + 1, LogParams.lenFile);

            auto len = sFile.length();
            if (len < LogParams.lenFile) {
                sFile.insert(len, LogParams.lenFile - len, 32);
            }
            sFile += " | "; Format += sFile;
        }
    }

    // Add function info
    if (LogParams.showFunction) {
        auto& sFunction = sLine;
        sFunction.assign(Function, 0, LogParams.lenFunction);

        auto len = sFunction.length();
        if (len < LogParams.lenFunction) {
            sFunction.insert(len, LogParams.lenFunction - len, 32);
        }
        sFunction += " | "; Format += sFunction;
    }

    // Add Line Info
    if (LogParams.showLine) {
        sLine = std::to_string(Line);
        const auto len = sLine.length();
        if (len < LogParams.lenLine) {
            Format.insert(Format.end(), LogParams.lenLine - len, 32);
        }
        sLine += " | "; Format += sLine;
    }

    Format += Fmt;

    va_list ap;
    va_start(ap, Fmt);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, sev, Format.c_str(), ap);
    va_end(ap);
#endif
}

Uint32 msgBox::GetEventCode()
{
    return CurrentContext.MessageBoxEvent;
}

void msgBox::Show(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, const char* Button4, Uint32 TimeOutMS)
{
    PlatformWrapper::GetInstance().MessageBoxShow(Code, Title, Message, Button1, Button2, Button3, Button4, TimeOutMS);
}

void msgBox::DecodeEvent(const SDL_Event& Event, int& Code, msgBox::Action& Action)
{
    Code = Event.user.code;
    Action = (msgBox::Action)(int)(size_t)Event.user.data1;
}

bool BWrapper::PrintDirContent(const char* Path, BWrapper::LogPriority Priority, bool Recursive)
{
    bool result = false;
    DirContentReader Dr;
    DirContentElement* Dc;

    if (Dr.Open(Path)) {
        result = true;
        while (Dr.Next(Dc)) {
            logVA(Priority, "%s %s/%s", (Dc->isDir ? "dir" : "file"), Path, Dc->Name.c_str());
            if (Recursive && Dc->isDir) {
                std::string p = std::string(Path) + "/" + Dc->Name;
                PrintDirContent(p.c_str(), Priority, Recursive);
            }
        }
    }
    Dr.Close();
    return result;
}

bool BWrapper::RandomInit()
{
    // https://ru.cppreference.com/w/cpp/numeric/random/rand
    std::srand(unsigned(std::time(nullptr)));
    return true;
};

int BWrapper::Random()
{
    return std::rand();
};

decltype(time(nullptr)) BWrapper::GetTimeSeconds()
{
    return time(nullptr);
}

std::string BWrapper::GetSDKVersionInfo()
{
    const auto versionToString = [](int Version) {
        return std::to_string(SDL_VERSIONNUM_MAJOR(Version)) + "." + std::to_string(SDL_VERSIONNUM_MINOR(Version)) + "." + std::to_string(SDL_VERSIONNUM_MICRO(Version));
    };
    return std::string("Compiled version: ") + versionToString(SDL_VERSION) + "; Linked version: " + versionToString(SDL_GetVersion());
}

bool BWrapper::DirExists(const char* Dir)
{
    return PlatformWrapper::GetInstance().DirExists(Dir);
}

//std::string BWrapper::GetInternalDir() { return Platforms::GetInternalDir(); };
std::string BWrapper::GetInternalWriteDir() { return PlatformWrapper::GetInstance().GetInternalWriteDir(); };

int BWrapper::GetAudioOutputRate() {
    return PlatformWrapper::GetInstance().GetAudioOutputRate();
}

int BWrapper::GetAudioOutputBufferSize() {
    return PlatformWrapper::GetInstance().GetAudioOutputBufferSize();
}

bool BWrapper::LaunchAppReviewIfAvailable() {
    return PlatformWrapper::GetInstance().LaunchAppReviewIfAvailable();
}

bool BWrapper::RequestFlexibleUpdateIfAvailable()
{
    return PlatformWrapper::GetInstance().RequestFlexibleUpdateIfAvailable();
}

bool DirContentReader::Next(DirContentElement*& Element)
{
    if (0 < Size && Pointer < Size)
    {
        Element = List[Pointer].get();
        ++Pointer;
        return true;
    }

    return false;
}

bool DirContentReader::Open(const char* Dir)
{
    Close();

    if (BWrapper::DirExists(Dir))
    {
        auto res = PlatformWrapper::GetInstance().GetDirContent(Dir, List);
        this->Size = List.size();
        return res;
    }

    return false;
}

/*
bool BWrapper::DirRemove(const char* Dir)
{
    return Platforms::DirRemove(Dir);
};
*/

bool BWrapper::DirRemoveRecursive(const char* Dir)
{
    auto res = PlatformWrapper::GetInstance().DirRemoveRecursive(Dir);
    return res && (!DirExists(Dir));
};

std::string BWrapper::GetEnvVariable(const char* Variable)
{
    return PlatformWrapper::GetInstance().GetEnvVariable(Variable);
}

void BWrapper::ShareText(const char* Title, const char* Message)
{
    PlatformWrapper::GetInstance().ShareText(Title, Message);
};

void BWrapper::SharePNG(const char* Title, const char* File) {
    PlatformWrapper::GetInstance().SharePNG(Title, File);
};

void BWrapper::SharePDF(const char* Title, const char* File) {
    PlatformWrapper::GetInstance().SharePDF(Title, File);
};


std::unique_ptr<std::istream> BWrapper::GetAssetStream(const char* Fname) {
    return PlatformWrapper::GetInstance().GetAssetStream(Fname);
}
//////////////////////////
/////// FileReader
//////////////////////////
bool FileReader::Open(const char* Fname, BWrapper::FileSearchPriority SearchPriority)
{
    Close();
    if (BWrapper::FileSearchPriority::Assets == SearchPriority) {
        in = PlatformWrapper::GetInstance().GetAssetStream(Fname);
    }
    else {
        in = std::make_unique<std::ifstream>(Fname, std::ios::binary | std::ios::in);
    }
    opened = (in ? true : false);
    return opened;
};

void FileReader::Close()
{
    opened = false;
    in.reset();
};

bool FileReader::ReadLine(std::string& Line)
{
    if (!opened)
    {
        logError("FileReader is closed");
        return false;
    }
    if (std::getline(*in, Line)) {
        if (!Line.empty()) {
            Line.erase(Line.find_last_not_of("\r\n") + 1);
        }
        return true;
    }
    return false;
}

bool FileReader::Read(char* Buffer, unsigned Size, unsigned& Readed)
{
    Readed = 0;
    if (!opened)
    {
        logError("FileReader is closed");
        return false;
    }
    in->read(Buffer, Size);
    Readed = (unsigned)in->gcount();
    return (Readed > 0);
}

void WAVPlayer::Clear()
{
    this->wav_length = 0;

    if (this->stream)
    {
        SDL_DestroyAudioStream(this->stream); // закрывает и устройство, открытое SDL_OpenAudioDeviceStream
        this->stream = nullptr;
    }

    if (this->wav_buffer)
    {
        SDL_free(this->wav_buffer);
        this->wav_buffer = nullptr;
    }
};

bool WAVPlayer::LoadFromMemory(const char* Buffer, int Size)
{
    this->Clear();

    if (nullptr == Buffer)
    {
        logError("Emptry buffer for wav load");
        return false;
    }

    bool result = true;
    auto io = SDL_IOFromMem((void*)Buffer, Size);
    if (!io)
    {
        logError("Error read SDL_IOFromMem %s", SDL_GetError());
        return false;
    }
    SDL_zero(this->wav_spec);

    if (!SDL_LoadWAV_IO(io, true, &this->wav_spec, &this->wav_buffer, &this->wav_length))
    {
        logError("error SDL_LoadWAV_IO %s", SDL_GetError());
        result = false;
    }
    else
    {
        // поток создается на паузе; данные поток сам конвертирует в формат устройства
        this->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &this->wav_spec, nullptr, nullptr);
        if (!this->stream)
        {
            logError("error SDL_OpenAudioDeviceStream %s", SDL_GetError());
            result = false;
        }
    }
    return result;
};

bool WAVPlayer::LoadFromFile(const char* FileName, const BWrapper::FileSearchPriority SearchPriority)
{
    this->Clear();
    auto fb = PlatformWrapper::GetInstance().GetFileBuf(FileName, SearchPriority);

    if (!fb) {
        logError("Error load file = %s, error=%s", FileName, SDL_GetError());
        return false;
    }

    if (!this->LoadFromMemory(fb->Begin(), fb->End() - fb->Begin())) {
        logError("Error load wav file = %s", FileName);
        return false;
    }

    return true;
};

bool WAVPlayer::Play()
{
    if (this->wav_length && this->stream)
    {
        //logDebug("Play audio");
        // как SDL_QueueAudio в SDL2: повторный Play ставит звук в очередь после текущего
        if (!SDL_PutAudioStreamData(this->stream, this->wav_buffer, static_cast<int>(this->wav_length)))
        {
            logError("error SDL_PutAudioStreamData %s", SDL_GetError());
            return false;
        }
        SDL_ResumeAudioStreamDevice(this->stream);
        return true;
    }
    logError("Error play wav: wav_length = 0");
    return false;
};