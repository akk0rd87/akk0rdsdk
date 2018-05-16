#include "basewrapper.h"
#include "SDL_image.h"
#include "core/platforms.h"
#include "core/core_defines.h"

//#include <stdio.h>
//#include <stdlib.h>

struct CurrentContextStruct
{
    AkkordRenderer*           CurrentRenderer = nullptr;
    AkkordWindow*             CurrentWindow = nullptr;
};

static CurrentContextStruct CurrentContext;
static LogParamsStruct LogParams;

bool BWrapper::Init(Uint32 flags)
{
    /*
    Если раскомментить нижеприведенную строку, то на винде валится при закрытии окна приложения (не консоли)
    if (SDL_SetMemoryFunctions(std::malloc, std::calloc, std::realloc, std::free) != 0)
        logError("SDL_SetMemoryFunctions error %s", SDL_GetError());
    */

    if (SDL_Init(flags) != 0)
    {
        logError("BWrapper::Init: Error %s", SDL_GetError());
        return false;
    }
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

    char mode[3];

    switch (OpenMode)
    {
        case BWrapper::FileOpenMode::ReadBinary:
            strcpy(mode, "rb");
            break;
        case BWrapper::FileOpenMode::WriteBinary:
            strcpy(mode, "wb");
            break;
        case BWrapper::FileOpenMode::AppendBinary:
            strcpy(mode, "ab");
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

    auto File = fopen(Filename, mode);
    return File;
}

char* File2Buffer_private(FILE* File, unsigned& Size)
{
    Size = 0;
    if(File)
    {
        fseek(File, 0, SEEK_END);
        Size = ftell(File);
        rewind(File); // set pointer to beginning
        char* buffer = new char[Size];
        auto readed = fread(buffer, Size, 1, File);
        if(readed != 1)
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
    if(!File) logError("BWrapper::FileOpen: File %s open error", FileName);
    return File;
}

char* BWrapper::File2Buffer(const char* FileName, FileSearchPriority SearchPriority, unsigned& BufferSize)
{
    char* buffer = nullptr;
    BufferSize  = 0;

    std::string Fname = std::string(FileName);

    if (FileSearchPriority::Assets == SearchPriority)
    {
#ifdef __ANDROID__ // На андроиде assets читаются особым образом
        buffer = AndroidWrapper::GetAsset2Buffer(Fname.c_str(), BufferSize);
        if (!buffer) logError("FileSearchPriority::Assets: File %s [%s] open error", FileName, Fname.c_str());
        return buffer;
#else
      //Во всех остальных случаях читаем из папки assets
        //Fname = std::string("assets/") + Fname;
        Fname = Platforms::GetInternalAssetsDir() + Fname;
#endif
    }

    auto File = FileOpen_private(Fname.c_str(), SearchPriority, BWrapper::FileOpenMode::ReadBinary);

    if(!File)
    {
        logError("BWrapper::File2Buffer: File %s open error", FileName, Fname.c_str());
        goto end;
    }

    buffer = File2Buffer_private(File, BufferSize);
    if (!buffer)
    {
        logError("BWrapper::File2Buffer: File %s read to buffer error", FileName, Fname.c_str());
    }

    end:
    FileClose(File);
    return buffer;
}


bool BWrapper::FileExists(const char* FileName, BWrapper::FileSearchPriority SearchPriority)
{
#ifdef __ANDROID__ // На андроиде assets читаются особым образом
    if (BWrapper::FileSearchPriority::Assets == SearchPriority)
    {
        unsigned BufferSize;
        auto buffer = AndroidWrapper::GetAsset2Buffer(FileName, BufferSize);
        if (buffer)
        {
            CloseBuffer(buffer);
            return true;
        }
        return false;
    }
#endif

    auto File = FileOpen_private(FileName, SearchPriority, BWrapper::FileOpenMode::ReadBinary);

    if(File)
    {
        FileClose(File);
        return true;
    }
    return false;
}


void BWrapper::CloseBuffer(char*& buffer)
{
    if (buffer)
    {
        delete [] buffer;
    }
    buffer = nullptr;
}

void BWrapper::FileClose(FILE*& File)
{
    if (File) fclose(File);
    File = nullptr;
}

bool BWrapper::FileWrite(FILE* File, const void* Buffer, size_t Size, size_t Count) // http://www.cplusplus.com/reference/cstdio/fwrite/
{
    if (fwrite(Buffer, Size, Count, File) == Count) return true;
    return false;
}

bool BWrapper::FileWriteFormatted(FILE* File, const char * Format, ...) // http://www.cplusplus.com/reference/cstdio/vfprintf/
{
    va_list args;
    va_start(args, Format);
    vfprintf(File, Format, args);
    va_end(args);

    return true;
}

bool BWrapper::FilePutS(FILE* File, const char * String) // http://www.cplusplus.com/reference/cstdio/fputs/
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

SDL_Rect ConvertRect2Native(const AkkordRect &Rect)
{
    SDL_Rect r;
    r.x = Rect.x; r.y = Rect.y; r.w = Rect.w; r.h = Rect.h;
    return r;
}

AkkordWindow* BWrapper::CreateRenderWindow(const char* Title, int X, int Y, int W, int H, Uint32 Flags)
{
    auto wnd = SDL_CreateWindow(Title, X, Y, W, H, Flags/* SDL_WINDOW_SHOWN*/);
    if(!wnd) logError("CreateWindow error = %s", SDL_GetError());
    return wnd;
};

AkkordRenderer* BWrapper::CreateRenderer(AkkordWindow* window, int index, Uint32 flags)
{
    auto rnd = SDL_CreateRenderer(window, index, flags);
    if (!rnd) logError("CreateRenderer error = %s", SDL_GetError());
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

AkkordRenderer*  BWrapper::GetActiveRenderer()
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
    SDL_DestroyRenderer(CurrentContext.CurrentRenderer);
    return true;
};

bool BWrapper::RefreshRenderer()
{
    SDL_RenderPresent(CurrentContext.CurrentRenderer);
    return true;
};

bool BWrapper::DestroyWindow()
{
    SDL_DestroyWindow(CurrentContext.CurrentWindow);
    return true;
};

bool BWrapper::SetWindowSize(int W, int H)
{
    SDL_SetWindowSize(CurrentContext.CurrentWindow, W, H);
    return true;
}

bool AkkordTexture::LoadFromFile(const char* FileName, TextureType Type, const BWrapper::FileSearchPriority SearchPriority)
{
    std::string Format;

    if (tex)
        this->Destroy();

    bool result = false;
    unsigned Size;
    auto buffer = BWrapper::File2Buffer(FileName, SearchPriority, Size);

    if (!buffer)
    {
        logError(std::string(Format + "Error load file image = %s, error=%s").c_str(), FileName, SDL_GetError());
        return result;
    }

    auto io = SDL_RWFromMem(buffer, Size);

    if (io)
    {
        SDL_Surface *image = nullptr;
        switch (Type)
        {
            case AkkordTexture::TextureType::BMP:
                image = IMG_LoadBMP_RW(io);
                break;
            case AkkordTexture::TextureType::PNG:
                image = IMG_LoadPNG_RW(io);
                break;
            case AkkordTexture::TextureType::JPEG:
                image = IMG_LoadJPG_RW(io);
                break;
        }

        if (image)
        {
            tex = SDL_CreateTextureFromSurface(CurrentContext.CurrentRenderer, image);
            SDL_FreeSurface(image);

            if (tex)
                result = true;
        }
        else
        {
            logError(std::string(Format + "Error load Image SDL_RWFromMem = %s, error=%s").c_str(), FileName, SDL_GetError());
        }
    }
    else
    {
        logError(std::string(Format + "Error load Image IMG_Load_RW = %s, error=%s").c_str(), FileName, SDL_GetError());
    }

    BWrapper::CloseBuffer(buffer);
    SDL_RWclose(io);

    return result;
};

void AkkordTexture::Destroy()
{
    if (tex)
    {
        SDL_DestroyTexture(tex);
    }
    tex = nullptr;
}

AkkordTexture::~AkkordTexture()
{
    Destroy();
}

const bool AkkordTexture::Draw(AkkordRect Rect, const AkkordRect* RectFromAtlas, unsigned char Flip, double Angle, AkkordPoint* Point)
{
    std::string Format;
    auto NativeDstRect = ConvertRect2Native(Rect); // Rect must be always set

    // Converting Source Rect if exists
    SDL_Rect  NativeSrcRect;
    SDL_Rect* NativeSrcRect_ptr = nullptr;
    if (RectFromAtlas)
    {
        NativeSrcRect     = ConvertRect2Native(*RectFromAtlas);
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

    auto res = SDL_RenderCopyEx(CurrentContext.CurrentRenderer, tex, NativeSrcRect_ptr, &NativeDstRect, Angle, point_ptr, flip);

    if (res != 0)
    {
        logError(std::string(Format + "Error draw image %s").c_str(), SDL_GetError());
        return false;
    }

    //SDL_BlendMode bmd;

    return true;
};

AkkordTexture::AkkordTexture()
{
    tex = nullptr;
}

const AkkordPoint AkkordTexture::GetSize()
{
    AkkordPoint Point(-1, -1);
    if (tex)
    {
        SDL_QueryTexture(tex, NULL, NULL, &Point.x, &Point.y);
    }
    else
    {
        logError("AkkordTexture::GetSize():  Texture is empty");
    }
    return Point;
};

AkkordPoint::AkkordPoint()
{
    x = 0; y = 0;
}

AkkordPoint::AkkordPoint(int X, int Y)
{
    x = X; y = Y;
}

AkkordPoint BWrapper::GetScreenSize()
{
    AkkordPoint WSize;
    //SDL_GetWindowSize(CurrentContext.CurrentWindow, &WSize.x, &WSize.y);
    //SDL_GL_GetDrawableSize(CurrentContext.CurrentWindow, &WSize.x, &WSize.y);
    SDL_GetRendererOutputSize(CurrentContext.CurrentRenderer, &WSize.x, &WSize.y);
    return WSize;
};

int BWrapper::GetScreenWidth()
{
    return GetScreenSize().x;
};

int BWrapper::GetScreenHeight()
{
    return GetScreenSize().y;
};

bool BWrapper::IsPointInRect(AkkordPoint Point, AkkordRect Rect)
{
    if (Rect.x <= Point.x && Point.x <= Rect.x + Rect.w)
        if (Rect.y <= Point.y && Point.y <= Rect.y + Rect.h)
            return true;

    return false;
}

void AkkordColor::SetInt32(unsigned int Color)
{
    color = Color;
};

AkkordColor::AkkordColor(unsigned int Color) // constructor
{
    color = Color;
};

AkkordColor::AkkordColor(unsigned char R, unsigned char G, unsigned char B) // constructor
{
    SetRGB(R, G, B);
};

AkkordColor::AkkordColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A) // constructor
{
    SetRGBA(R, G, B, A);
};

AkkordColor::AkkordColor()
{
    SetRGB(255, 255, 255);
}

void AkkordColor::SetRGB(unsigned char R, unsigned char G, unsigned char B)
{
    SetRGBA(R, G, B, 255);
};

void AkkordColor::SetRGBA(unsigned char R, unsigned char G, unsigned char B, unsigned char A)
{
    color = AkkordColor::RGBA2Int32(R, G, B, A);
};

unsigned char AkkordColor::GetRFromInt32(unsigned int ColorInt32)
{
    return (ColorInt32 & 0x000000ff);
};

unsigned char AkkordColor::GetGFromInt32(unsigned int ColorInt32)
{
    return (ColorInt32 & 0x0000ff00) >> 8;
};

unsigned char AkkordColor::GetBFromInt32(unsigned int ColorInt32)
{
    return (ColorInt32 & 0x00ff0000) >> 16;
};

unsigned char AkkordColor::GetAFromInt32(unsigned int ColorInt32)
{
    return (ColorInt32 & 0xff000000) >> 24;
};

const unsigned char AkkordColor::GetR()
{
    return AkkordColor::GetRFromInt32(color);
};

const unsigned char AkkordColor::GetG()
{
    return AkkordColor::GetGFromInt32(color);
};

const unsigned char AkkordColor::GetB()
{
    return AkkordColor::GetBFromInt32(color);
};

const unsigned char AkkordColor::GetA()
{
    return AkkordColor::GetAFromInt32(color);;
};

const unsigned int AkkordColor::GetInt32()
{
    return color;
};

unsigned int AkkordColor::RGBA2Int32(int r, int g, int b, int a)
{
    return r
        + g * 256
        + b * 256 * 256
        + a * 256 * 256 * 256;
}

bool BWrapper::SetCurrentColor(AkkordColor Color)
{
    if(SDL_SetRenderDrawColor(CurrentContext.CurrentRenderer, Color.GetR(), Color.GetG(), Color.GetB(), Color.GetA()) == 0) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
}

bool BWrapper::DrawRect(AkkordRect Rect)
{
    auto NativeRect = ConvertRect2Native(Rect);
    if (SDL_RenderDrawRect(CurrentContext.CurrentRenderer, &NativeRect) == 0) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

bool BWrapper::FillRect(AkkordRect Rect)
{
    auto NativeRect = ConvertRect2Native(Rect);
    if (SDL_RenderFillRect(CurrentContext.CurrentRenderer, &NativeRect) == 0) return true;
    logError("Draw error %s", SDL_GetError());
    return false;
};

bool BWrapper::FillRect(int X, int Y, int W, int H)
{
    return BWrapper::FillRect(AkkordRect(X, Y, W, H));
}

bool BWrapper::DrawLine(AkkordPoint Point1, AkkordPoint Point2)
{
    if (SDL_RenderDrawLine(CurrentContext.CurrentRenderer, Point1.x, Point1.y, Point2.x, Point2.y) == 0) return true;
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

BWrapper::KeyCodes BWrapper::DecodeKey(SDL_Keysym SDL_Key)
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

        default: return KeyCodes::Uknown;
    }
}

/*
void BWrapper::MessageBoxSetColorScheme(MessageBoxColorScheme& Scheme)
{
    MessageBoxColorScheme2Native(&Scheme, CurrentContext.MessageBoxColorScheme);
}
*/

int BWrapper::ShowMessageBox(const char* Message)
{
    // https://wiki.libsdl.org/SDL_ShowMessageBox
    // https://wiki.libsdl.org/SDL_ShowSimpleMessageBox
    const SDL_MessageBoxButtonData buttons[] = {
        { /* .flags, .buttonid, .text */        0, 0, "Ok" },
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "yes" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "cancel" },
    };
    const SDL_MessageBoxColorScheme colorScheme = {
        { /* .colors (.r, .g, .b) */
            /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
            { 88, 135, 63 },
            /* [SDL_MESSAGEBOX_COLOR_TEXT] */
            { 250, 250, 250 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
            { 255, 255, 0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
            { 0, 0, 255 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
            { 255, 0, 255 }
        }
    };

    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION, /* .flags */
        //NULL, /* .window */
        CurrentContext.CurrentWindow,
        //NULL,
        "example message box", /* .title */
        Message, /* .message */
        SDL_arraysize(buttons), /* .numbuttons */
        buttons, /* .buttons */
        &colorScheme /* .colorScheme */
    };

    int buttonid;

    if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
        SDL_Log("error displaying message box");
        return 1;
    }
    if (buttonid == -1) {
        SDL_Log("no selection");
    }
    else {
        SDL_Log("selection was %s", buttons[buttonid].text);
    }

    return 0; // Заглушка
}

/*
int BWrapper::GetDisplayDPI(float* Ddpi, float* Hdpi, float* Vdpi)
{
    return SDL_GetDisplayDPI(0 Display Index , Ddpi, Hdpi, Vdpi);
}
*/

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
    //return Platforms::GetDeviceLanguage();
    return Locale::Lang::Unknown;
};


BWrapper::OS BWrapper::GetDeviceOS()
{
    return Platforms::GetDeviceOS();
};

int BWrapper::AndroidGetApiLevel()
{
    return Platforms::AndroidGetApiLevel();
}

bool BWrapper::AndroidShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset)
{
    return Platforms::AndroidShowToast(Message, Duration, Gravity, xOffset, yOffset);
}

void BWrapper::SetLogPriority(BWrapper::LogPriority Priority)
{
    SDL_LogPriority sev = (SDL_LogPriority)Priority;
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, sev);
}

LogParamsStruct* BWrapper::GetLogParams()
{
    return &LogParams;
}

void BWrapper::Log(BWrapper::LogPriority Priority, const char* File, const char* Function, unsigned Line, SDL_PRINTF_FORMAT_STRING const char *Fmt, ...)
{
    //#if defined(DEBUG) || defined(_DEBUG) || defined(_DEBUG_) || defined(__DEBUG__) || defined(NDK_DEBUG)
    // https://wiki.libsdl.org/CategoryLog
    SDL_LogPriority sev = (SDL_LogPriority)Priority;
    char TimeBuffer[16];

    auto ms = BWrapper::GetTicks();
    auto hh24 = ms / 60 / 60 / 1000;
    ms = ms - hh24 * 60 / 60 / 1000;
    auto mi = ms / 60 / 1000;
    ms = ms - mi * 60 * 1000;
    auto ss = ms / 1000;
    ms = ms % 1000;

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

    auto Format = std::string(TimeBuffer);

    // Add File Info
    if (LogParams.showFile)
    {
        unsigned pos = 0;
        const char* p = File;
        for (unsigned i = 0; (*p); ++p, ++i)
            if((*p) == '/' || (*p) == '\\')
                pos = i + 1;

        std::string sFile = std::string(File, pos, std::string::npos);

        auto len = sFile.length();
        if (len < LogParams.lenFile)
            sFile.insert(len, LogParams.lenFile - len, 32);

        sFile = sFile + " | ";
        Format = Format + sFile;
    }

    // Add function info
    if (LogParams.showFunction)
    {
        std::string sFunction = std::string(Function);

        auto len = sFunction.length();
        if (len < LogParams.lenFunction)
            sFunction.insert(len, LogParams.lenFunction - len, 32);

        sFunction = sFunction + " | ";
        Format = Format + sFunction;
    }

    // Add Line Info
    if (LogParams.showLine)
    {
        std::string sLine = std::to_string(Line);

        auto len = sLine.length();
        if (len < LogParams.lenLine)
            sLine.insert(len, LogParams.lenLine - len, 32);

        sLine = sLine + " | ";
        Format = Format + sLine;
    }

    int len = 0;
    switch (Priority)
    {
        case BWrapper::LogPriority::Verbose : len = 7; break;
        case BWrapper::LogPriority::Debug   : len = 5; break;
        case BWrapper::LogPriority::Info    : len = 4; break;
        case BWrapper::LogPriority::Warning : len = 4; break; /* base WARN*/
        case BWrapper::LogPriority::Error   : len = 5; break;
        case BWrapper::LogPriority::Critical: len = 8; break;
    }

    Format = Format + Fmt;
    Format.insert(0, 8 - len, 32);

    va_list ap;
    va_start(ap, Fmt);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, sev, Format.c_str(), ap);
    va_end(ap);
    //#endif
}

bool BWrapper::PrintDirContent(const char* Path, BWrapper::LogPriority Priority, bool Recursive)
{
    bool result = false;

    const char* file = "[file]";
    const char* dir  = "[dir ]";
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

            log(Priority, "%s %s/%s", ptype, Path, Dc->Name.c_str());

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
    srand((unsigned)time(NULL));
    return true;
};

int BWrapper::Random()
{
    return rand();
};


decltype(time(NULL)) BWrapper::GetTimeSeconds()
{
    return time(NULL);
}

unsigned BWrapper::GetTicks()
{
    return SDL_GetTicks();
}

void BWrapper::Sleep(unsigned MilliSeconds)
{
    SDL_Delay(MilliSeconds);
}

AkkordRect::AkkordRect()
{
    x = 0; y = 0; w = 0; h = 0;
}

AkkordRect::AkkordRect(int X, int Y, int W, int H)
{
    x = X; y = Y; w = W; h = H;
};

AkkordRect::AkkordRect(AkkordPoint Point1, AkkordPoint Point2)
{
    x = Point1.x; y = Point1.y; w = Point2.x; h = Point2.y;
}

bool BWrapper::DirExists(const char* Dir)
{
    return Platforms::DirExists(Dir);
}

std::string BWrapper::GetInternalDir()      { return Platforms::GetInternalDir(); };
std::string BWrapper::GetInternalWriteDir() { return Platforms::GetInternalWriteDir(); };


//
bool DirContentReader::Close()
{
    List.clear();
    this->Pointer = 0;
    this->Size = 0;

    return true;
}

DirContentReader::~DirContentReader()
{
    Close();
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
//////////////////////////
/////// FileReader
//////////////////////////
bool FileReader::Open(const char *Fname, BWrapper::FileSearchPriority SearchPriority)
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
    if (fb.open(Path.c_str(), std::ios::binary | std::ios::in))
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

FileReader::FileReader()
{
    Close();
};

FileReader::~FileReader()
{
    Close();
};

const bool FileReader::IsOpen()
{
    return opened;
}

bool FileReader::ReadLine(std::string& Line)
{
    if (!opened)
    {
        logError("FileReader is closed");
        return false;
    }
    if (std::getline(*in, Line))
    {
        auto len = Line.size();

        while (len && (Line[len - 1] == '\n' || Line[len - 1] == '\r'))
        {
            Line = std::string(Line, 0, len - 1);
            len = Line.size();
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
