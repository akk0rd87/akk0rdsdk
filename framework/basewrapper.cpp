#include "basewrapper.h"
#include "SDL_image.h"
#include "core/platforms.h"
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

    if (SDL_Init(flags) != 0)
    {
        logError("BWrapper::Init: Error %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);

    CurrentContext.MessageBoxEvent = SDL_RegisterEvents(1);

    Platforms::Init();

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

char* File2Buffer_private(FILE* File, unsigned& Size)
{
    Size = 0;
    if (File)
    {
        fseek(File, 0, SEEK_END);
        Size = ftell(File);
        rewind(File); // set pointer to beginning
        char* buffer = new char[Size];
        auto readed = fread(buffer, Size, 1, File);
        if (readed != 1)
        {
            BWrapper::CloseBuffer(buffer);
            logError("File2Buffer_private: File read into buffer error. Readed %ld", readed);
            return nullptr;
        }
        return buffer;
    }
    logError("File2Buffer_private: Empty file handle");
    return nullptr;
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

char* BWrapper::File2Buffer(const char* FileName, FileSearchPriority SearchPriority, unsigned& BufferSize)
{
    char* buffer = nullptr;
    BufferSize = 0;

    std::string Fname(FileName);

    if (FileSearchPriority::Assets == SearchPriority)
    {
#ifdef __ANDROID__ // На андроиде assets читаются особым образом
        buffer = AndroidWrapper::GetAsset2Buffer(Fname.c_str(), BufferSize);

        if (nullptr == buffer)
            logError("FileSearchPriority::Assets: File %s [%s] open error", FileName, Fname.c_str());

        return buffer;
#else
        //Во всех остальных случаях читаем из папки assets
        Fname = Platforms::GetInternalAssetsDir() + std::string(FileName);
#endif
    }

    std::unique_ptr<FILE, int(*)(FILE*)> File(FileOpen_private(Fname.c_str(), SearchPriority, BWrapper::FileOpenMode::ReadBinary), fclose);

    if (nullptr == File)
    {
        logError("BWrapper::File2Buffer: File %s open error", FileName, Fname.c_str());
    }

    buffer = File2Buffer_private(File.get(), BufferSize);
    if (nullptr == buffer)
    {
        logError("BWrapper::File2Buffer: File %s read to buffer error", FileName, Fname.c_str());
    }
    return buffer;
}

bool BWrapper::FileExists(const char* FileName, BWrapper::FileSearchPriority SearchPriority)
{
#ifdef __ANDROID__ // На андроиде assets читаются особым образом
    if (BWrapper::FileSearchPriority::Assets == SearchPriority)
    {
        unsigned BufferSize;
        auto buffer = AndroidWrapper::GetAsset2Buffer(FileName, BufferSize);
        if (buffer != nullptr)
        {
            CloseBuffer(buffer);
            return true;
        }
        return false;
    }
#endif

    auto File = FileOpen_private(FileName, SearchPriority, BWrapper::FileOpenMode::ReadBinary);

    if (File != nullptr)
    {
        FileClose(File);
        return true;
    }
    return false;
}

void BWrapper::CloseBuffer(char*& buffer)
{
    if (buffer != nullptr)
    {
        delete[] buffer;
    }
    buffer = nullptr;
}

void BWrapper::FileClose(FILE*& File)
{
    if (File != nullptr)
    {
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
    return Platforms::DirCreate(Dir);
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

void inline ConvertRect2Native(const AkkordRect& Rect, SDL_Rect* sRect)
{
    sRect->x = Rect.x; sRect->y = Rect.y; sRect->w = Rect.w; sRect->h = Rect.h;
}

AkkordWindow* BWrapper::CreateRenderWindow(const char* Title, int X, int Y, int W, int H, Uint32 Flags)
{
    auto wnd = SDL_CreateWindow(Title, X, Y, W, H, Flags/* SDL_WINDOW_SHOWN*/);
    if (nullptr == wnd)
        logError("CreateWindow error = %s", SDL_GetError());
    return wnd;
};

AkkordRenderer* BWrapper::CreateRenderer(AkkordWindow* window, int index, Uint32 flags)
{
    auto rnd = SDL_CreateRenderer(window, index, flags);
    if (nullptr == rnd)
        logError("CreateRenderer error = %s", SDL_GetError());
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
    SDL_SetWindowResizable(CurrentContext.CurrentWindow, true == Resizable ? SDL_TRUE : SDL_FALSE);
    return true;
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
    SDL_RenderFlush(CurrentContext.CurrentRenderer);
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
    if (nullptr == Buffer)
    {
        logError("Error load texture from memory: buffer is not specified");
        return false;
    }

    std::unique_ptr<SDL_RWops, void(*)(SDL_RWops*)>io(SDL_RWFromMem((void*)Buffer, Size), [](SDL_RWops* i) {SDL_RWclose(i); });
    bool result = false;

    if (io)
    {
        std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)> image(nullptr, SDL_FreeSurface);
        switch (Type)
        {
        case AkkordTexture::TextureType::BMP:
            image = std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>(IMG_LoadBMP_RW(io.get()), SDL_FreeSurface);
            break;
        case AkkordTexture::TextureType::PNG:
            image = std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>(IMG_LoadPNG_RW(io.get()), SDL_FreeSurface);
            break;
        case AkkordTexture::TextureType::JPEG:
            image = std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>(IMG_LoadJPG_RW(io.get()), SDL_FreeSurface);
            break;
        case AkkordTexture::TextureType::SVG:
        {
            std::unique_ptr<char, void(*)(char*)> data((char*)SDL_LoadFile_RW(io.get(), nullptr, SDL_FALSE), [](char* i) { SDL_free(i); });
            if (data.get() == nullptr)
            {
                logError("Couldn't parse SVG image %s", SDL_GetError());
                return result;
            }
            std::unique_ptr<NSVGimage, void(*)(NSVGimage*)> svg_image(nsvgParse(data.get(), "px", 96.0f), nsvgDelete);

            if (svg_image.get() == nullptr)
            {
                logError("Couldn't parse SVG image %s", SDL_GetError());
                return result;
            }

            std::unique_ptr<NSVGrasterizer, void(*)(NSVGrasterizer*)>rasterizer(nsvgCreateRasterizer(), nsvgDeleteRasterizer);
            if (rasterizer.get() == nullptr)
            {
                logError("Couldn't create SVG rasterizer %s", SDL_GetError());
                return result;
            }

            image = std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>(
                SDL_CreateRGBSurface(SDL_SWSURFACE,
                    (int)(svg_image->width * Scale),
                    (int)(svg_image->height * Scale),
                    32,
                    0x000000FF,
                    0x0000FF00,
                    0x00FF0000,
                    0xFF000000),
                SDL_FreeSurface);

            if (image.get() == nullptr)
            {
                logError("Couldn't create SDL_CreateRGBSurface %s", SDL_GetError());
                return result;
            }

            nsvgRasterize(rasterizer.get(), svg_image.get(), 0.0f, 0.0f, Scale, (unsigned char*)image->pixels, image->w, image->h, image->pitch);
        }
        break;
        }

        if (image.get())
        {
            result = this->CreateFromSurface(image.get());
        }
        else
        {
            logError("Error load Image SDL_RWFromMem error=%s", SDL_GetError());
        }
    }

    return result;
};

bool AkkordTexture::LoadFromFile(const char* FileName, TextureType Type, const BWrapper::FileSearchPriority SearchPriority, float Scale)
{
    if (tex != nullptr)
    {
        this->Destroy();
    }

    bool result = false;
    unsigned Size;
    auto buffer = BWrapper::File2Buffer(FileName, SearchPriority, Size);

    if (nullptr == buffer)
    {
        logError("Error load file image = %s, error=%s", FileName, SDL_GetError());
        return result;
    }

    result = this->LoadFromMemory(buffer, Size, Type, Scale);
    BWrapper::CloseBuffer(buffer);

    if (!result)
    {
        logError("Error load file %s", FileName);
    }

    return result;
};

bool AkkordTexture::SetColorMod(Uint8 R, Uint8 G, Uint8 B)
{
    if (SDL_SetTextureColorMod(tex.get(), R, G, B) == 0)
        return true;

    logError("SDL_SetTextureColorMod error: %s", SDL_GetError());
    return false;
}

bool AkkordTexture::SetAlphaMod(Uint8 A)
{
    if (SDL_SetTextureAlphaMod(tex.get(), A) == 0)
        return true;

    logError("SDL_SetTextureAlphaMod error: %s", SDL_GetError());
    return false;
};

bool AkkordTexture::Draw(const AkkordRect& Rect, const AkkordRect* RectFromAtlas) const
{
    SDL_Rect  NativeDstRect, NativeSrcRect;
    SDL_Rect* NativeSrcRect_ptr = nullptr;

    ConvertRect2Native(Rect, &NativeDstRect); // Rect must be always set

    if (RectFromAtlas)
    {
        ConvertRect2Native(*RectFromAtlas, &NativeSrcRect);
        NativeSrcRect_ptr = &NativeSrcRect;
    }

    auto res = SDL_RenderCopy(CurrentContext.CurrentRenderer, tex.get(), NativeSrcRect_ptr, &NativeDstRect);

    if (res != 0)
    {
        logError("Error draw image %s", SDL_GetError());
        return false;
    }

    return true;
};

bool AkkordTexture::Draw(const AkkordRect& Rect, const AkkordRect* RectFromAtlas, unsigned char Flip, double Angle, AkkordPoint* Point) const
{
    SDL_Rect NativeDstRect;
    ConvertRect2Native(Rect, &NativeDstRect); // Rect must be always set

    // Converting Source Rect if exists
    SDL_Rect  NativeSrcRect;
    SDL_Rect* NativeSrcRect_ptr = nullptr;
    if (RectFromAtlas)
    {
        ConvertRect2Native(*RectFromAtlas, &NativeSrcRect);
        NativeSrcRect_ptr = &NativeSrcRect;
    }

    // Converting Angle Point if exists
    SDL_Point point;
    SDL_Point* point_ptr = nullptr;

    if (Point)
    {
        point.x = Point->x;
        point.y = Point->y;
        point_ptr = &point;
    }

    // converting Flip
    const SDL_RendererFlip flip = (SDL_RendererFlip)Flip;

    auto res = SDL_RenderCopyEx(CurrentContext.CurrentRenderer, tex.get(), NativeSrcRect_ptr, &NativeDstRect, Angle, point_ptr, flip);

    if (res != 0)
    {
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
        SDL_QueryTexture(tex.get(), nullptr, nullptr, &Point.x, &Point.y);
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
    SDL_GetRendererOutputSize(CurrentContext.CurrentRenderer, &WSize.x, &WSize.y);
    return WSize;
};

bool BWrapper::SetCurrentColor(const AkkordColor& Color)
{
    if (SDL_SetRenderDrawColor(CurrentContext.CurrentRenderer, Color.GetR(), Color.GetG(), Color.GetB(), Color.GetA()) == 0) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
}

bool BWrapper::DrawRect(const AkkordRect& Rect)
{
    SDL_Rect NativeRect;
    ConvertRect2Native(Rect, &NativeRect);
    if (SDL_RenderDrawRect(CurrentContext.CurrentRenderer, &NativeRect) == 0) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

bool BWrapper::DrawRect(int X, int Y, int W, int H)
{
    return DrawRect(AkkordRect(X, Y, W, H));
}

bool BWrapper::FillRect(const AkkordRect& Rect)
{
    SDL_Rect NativeRect;
    ConvertRect2Native(Rect, &NativeRect);
    if (SDL_RenderFillRect(CurrentContext.CurrentRenderer, &NativeRect) == 0) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

bool BWrapper::FillRect(int X, int Y, int W, int H)
{
    return BWrapper::FillRect(AkkordRect(X, Y, W, H));
}

bool BWrapper::DrawLine(const AkkordPoint& Point1, const AkkordPoint& Point2)
{
    if (SDL_RenderDrawLine(CurrentContext.CurrentRenderer, Point1.x, Point1.y, Point2.x, Point2.y) == 0) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

bool BWrapper::DrawLine(int P1X, int P1Y, int P2X, int P2Y)
{
    return DrawLine(AkkordPoint(P1X, P1Y), AkkordPoint(P2X, P2Y));
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

BWrapper::KeyCodes BWrapper::DecodeKey(const SDL_Keysym& SDL_Key)
{
    //enum struct KeyCodes { Esc, BackSpace, Back, Enter, Tab, Delete, F1,
    // Help, Home, Insert, Find, Copy, PageDown, PageUp, Paste, Pause, PrintScreen, Return, Space, Uknown };

    switch (SDL_Key.sym)
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
    case SDLK_QUOTE:        return KeyCodes::Quote;

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

    case SDLK_a: return KeyCodes::A;
    case SDLK_b: return KeyCodes::B;
    case SDLK_c: return KeyCodes::C;
    case SDLK_d: return KeyCodes::D;
    case SDLK_e: return KeyCodes::E;
    case SDLK_f: return KeyCodes::F;
    case SDLK_g: return KeyCodes::G;
    case SDLK_h: return KeyCodes::H;
    case SDLK_i: return KeyCodes::I;
    case SDLK_j: return KeyCodes::J;
    case SDLK_k: return KeyCodes::K;
    case SDLK_l: return KeyCodes::L;
    case SDLK_m: return KeyCodes::M;
    case SDLK_n: return KeyCodes::N;
    case SDLK_o: return KeyCodes::O;
    case SDLK_p: return KeyCodes::P;
    case SDLK_q: return KeyCodes::Q;
    case SDLK_r: return KeyCodes::R;
    case SDLK_s: return KeyCodes::S;
    case SDLK_t: return KeyCodes::T;
    case SDLK_u: return KeyCodes::U;
    case SDLK_v: return KeyCodes::V;
    case SDLK_w: return KeyCodes::W;
    case SDLK_x: return KeyCodes::X;
    case SDLK_y: return KeyCodes::Y;
    case SDLK_z: return KeyCodes::Z;

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
    return SDL_GetDisplayDPI(DisplayIndex, Ddpi, Hdpi, Vdpi);
}

void BWrapper::Quit()
{
    IMG_Quit();
    SDL_Quit();
    //atexit(SDL_Quit);
    //atexit(IMG_Quit);
}

bool BWrapper::OpenURL(const char* url)
{
    if (Platforms::OpenURL(url))
        return true;

    logError("URL %s was not opened", url);
    return false;
};

Locale::Lang BWrapper::GetDeviceLanguage()
{
    return Platforms::GetDeviceLanguage();
    //return Locale::Lang::Unknown;
};

//BWrapper::OS BWrapper::GetDeviceOS()
//{
//    return Platforms::GetDeviceOS();
//};

int BWrapper::AndroidGetApiLevel()
{
    return Platforms::AndroidGetApiLevel();
}

bool BWrapper::AndroidShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset)
{
    return Platforms::AndroidShowToast(Message, Duration, Gravity, xOffset, yOffset);
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

void msgBox::Show(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, Uint32 TimeOutMS)
{
    Platforms::MessageBoxShow(Code, Title, Message, Button1, Button2, Button3, TimeOutMS);
}

void msgBox::DecodeEvent(const SDL_Event& Event, int& Code, msgBox::Action& Action)
{
    Code = Event.user.code;
    Action = (msgBox::Action)(int)(size_t)Event.user.data1;
}

bool BWrapper::PrintDirContent(const char* Path, BWrapper::LogPriority Priority, bool Recursive)
{
    bool result = false;

    const char* file = "[file]";
    const char* dir = "[dir ]";
    const char* ptype;
    DirContentReader Dr;
    DirContentElement* Dc;

    if (Dr.Open(Path))
    {
        result = true;
        while (Dr.Next(Dc))
        {
            if (Dc->isDir) ptype = dir;
            else           ptype = file;

            logVA(Priority, "%s %s/%s", ptype, Path, Dc->Name.c_str());

            if (Recursive && Dc->isDir)
            {
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
    std::string VersionString;
    SDL_version version;
    SDL_VERSION(&version);
    VersionString = std::string("Compiled version: ") + std::to_string(version.major) + "." + std::to_string(version.minor) + std::to_string(version.patch) + "; ";

    SDL_GetVersion(&version);
    VersionString += std::string("Linked version: ") + std::to_string(version.major) + "." + std::to_string(version.minor) + std::to_string(version.patch) + ", Revision: " + SDL_GetRevision();
    return VersionString;
}

bool BWrapper::DirExists(const char* Dir)
{
    return Platforms::DirExists(Dir);
}

std::string BWrapper::GetInternalDir() { return Platforms::GetInternalDir(); };
std::string BWrapper::GetInternalWriteDir() { return Platforms::GetInternalWriteDir(); };

int BWrapper::GetAudioOutputRate() {
    return Platforms::GetAudioOutputRate();
}

int BWrapper::GetAudioOutputBufferSize() {
    return Platforms::GetAudioOutputBufferSize();
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
        auto res = Platforms::GetDirContent(Dir, List);
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
    auto res = Platforms::DirRemoveRecursive(Dir);
    return res && (!DirExists(Dir));
};

std::string BWrapper::GetEnvVariable(const char* Variable)
{
    return Platforms::GetEnvVariable(Variable);
}

void BWrapper::ShareText(const char* Title, const char* Message)
{
    Platforms::ShareText(Title, Message);
};

void BWrapper::SharePNG(const char* Title, const char* File) {
    Platforms::SharePNG(Title, File);
};

//////////////////////////
/////// FileReader
//////////////////////////
bool FileReader::Open(const char* Fname, BWrapper::FileSearchPriority SearchPriority)
{
    Close();
    std::string Path;
#ifdef __ANDROID__
    if (BWrapper::FileSearchPriority::Assets == SearchPriority)
    {
        unsigned Size;
        buffer = BWrapper::File2Buffer(Fname, SearchPriority, Size);

        if (buffer)
        {
            sbuf = new membuf(buffer, buffer + Size);
            in = new std::istream(sbuf);
            opened = true;
        }
        return opened;
    }
#else
    if (BWrapper::FileSearchPriority::Assets == SearchPriority)
        Path = Platforms::GetInternalAssetsDir() + "/";
#endif

    Path = Path + Fname;
    if (fb.open(Path.c_str(), std::ios::binary | std::ios::in) != nullptr)
    {
        in = new std::istream(&fb);
        opened = true;
    }
    return opened;
    };

void FileReader::Close()
{
    opened = false;
    fb.close();
    if (in) delete in;
    in = nullptr;

#ifdef __ANDROID__
    BWrapper::CloseBuffer(buffer);
    buffer = nullptr;

    if (sbuf) delete sbuf;
    sbuf = nullptr;
#endif
};

bool FileReader::ReadLine(std::string& Line)
{
    if (!opened)
    {
        logError("FileReader is closed");
        return false;
    }
    if (std::getline(*in, Line)) {
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

    if (this->deviceId)
    {
        SDL_CloseAudioDevice(this->deviceId);
        this->deviceId = 0;
    }

    if (this->wav_buffer)
    {
        SDL_FreeWAV(this->wav_buffer);
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
    auto io = SDL_RWFromMem((void*)Buffer, Size);
    if (!io)
    {
        logError("Error read SDL_RWFromMem %s", SDL_GetError());
        return false;
    }
    SDL_zero(this->wav_spec);

    if (SDL_LoadWAV_RW(io, 1, &this->wav_spec, &this->wav_buffer, &this->wav_length) == nullptr)
    {
        logError("error SDL_LoadWAV_RW %s", SDL_GetError());
        result = false;
    }
    else
    {
        this->deviceId = SDL_OpenAudioDevice(nullptr, 0, &this->wav_spec, nullptr, 0);
        //logDebug("deviceId = %d", deviceId);
    }
    return result;
};

bool WAVPlayer::LoadFromFile(const char* FileName, const BWrapper::FileSearchPriority SearchPriority)
{
    this->Clear();
    unsigned Size;
    auto buffer = BWrapper::File2Buffer(FileName, BWrapper::FileSearchPriority::Assets, Size);

    if (nullptr == buffer)
    {
        logError("Error load file = %s, error=%s", FileName, SDL_GetError());
        return false;
    }

    bool result = this->LoadFromMemory(buffer, Size);
    BWrapper::CloseBuffer(buffer);

    if (!result)
    {
        logError("Error load wav file = %s, error=%s", FileName, SDL_GetError());
    }

    return result;
};

bool WAVPlayer::Play()
{
    if (this->wav_length)
    {
        //logDebug("Play audio");
        SDL_QueueAudio(this->deviceId, this->wav_buffer, this->wav_length);
        SDL_PauseAudioDevice(this->deviceId, 0);
        return true;
    }
    logError("Error play wav: wav_length = 0");
    return false;
};