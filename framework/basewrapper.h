#pragma once
#ifndef __AKK0RD_BASE_BASEWRAPPER_H__
#define __AKK0RD_BASE_BASEWRAPPER_H__

#define _CRT_SECURE_NO_WARNINGS

#include "framework_defines.h"
#include "core/locale.h"

class AkkordPoint
{
public:
    int x, y;
    AkkordPoint() : x(0), y(0) {};
    AkkordPoint(int X, int Y) : x(X), y(Y) {};
    AkkordPoint(const AkkordPoint& Point) : x(Point.x), y(Point.y) {};
};

class AkkordRect
{
public:
    int x, y, w, h;
    AkkordRect() : x(0), y(0), w(0), h(0) {};
    AkkordRect(const AkkordRect& Rect) : x(Rect.x), y(Rect.y), w(Rect.w), h(Rect.h) {};
    AkkordRect(int X, int Y, int W, int H) : x(X), y(Y), w(W), h(H) {};
    AkkordRect(const AkkordPoint& Point1, const AkkordPoint& Point2) : x(Point1.x), y(Point1.y), w(Point2.x), h(Point2.y) {};

    void SetW(int W) { w = W; };
    void SetH(int H) { h = H; };
    void SetX(int X) { x = X; };
    void SetY(int Y) { y = Y; };

    void SetPosition(const AkkordPoint& Position) { x = Position.x; y = Position.y; };
    void SetSize(const AkkordPoint& Size) { w = Size.x; h = Size.y; };

    AkkordPoint GetPosition() const { return AkkordPoint(x, y); }
    AkkordPoint GetSize() const { return AkkordPoint(w, h); }
};

class AkkordColor
{
private:
    Uint32 color;
public:
    AkkordColor() : color{ 0 } {};
    AkkordColor(Uint32 Color) : color(Color) {};
    AkkordColor(Uint8 R, Uint8 G, Uint8 B) { SetRGB(R, G, B); };
    AkkordColor(Uint8 R, Uint8 G, Uint8 B, Uint8 A) { SetRGBA(R, G, B, A); };

    void                 SetInt32(Uint32 Color) { color = Color; };
    void                 SetRGB(Uint8 R, Uint8 G, Uint8 B) { SetRGBA(R, G, B, static_cast<Uint8>(255)); };
    void                 SetRGBA(Uint8 R, Uint8 G, Uint8 B, Uint8 A) { color = AkkordColor::RGBA2Int32(R, G, B, A); };

    void                 SetR(Uint8 R) { color &= static_cast<Uint32>(0xffffff00); color |= static_cast<Uint32>(R); };
    void                 SetG(Uint8 G) { color &= static_cast<Uint32>(0xffff00ff); color |= (static_cast<Uint32>(G) << 8); };
    void                 SetB(Uint8 B) { color &= static_cast<Uint32>(0xff00ffff); color |= (static_cast<Uint32>(B) << 16); };
    void                 SetA(Uint8 A) { color &= static_cast<Uint32>(0x00ffffff); color |= (static_cast<Uint32>(A) << 24); };

    Uint32               GetInt32() const { return color; };
    Uint8                GetR() const { return AkkordColor::GetRFromInt32(color); };
    Uint8                GetG() const { return AkkordColor::GetGFromInt32(color); };
    Uint8                GetB() const { return AkkordColor::GetBFromInt32(color); };
    Uint8                GetA() const { return AkkordColor::GetAFromInt32(color); };

    static constexpr Uint8 GetRFromInt32(Uint32 ColorInt32) { return static_cast<Uint8>((ColorInt32 & static_cast<Uint32>(0x000000ff))); };
    static constexpr Uint8 GetGFromInt32(Uint32 ColorInt32) { return static_cast<Uint8>((ColorInt32 & static_cast<Uint32>(0x0000ff00)) >> 8); };
    static constexpr Uint8 GetBFromInt32(Uint32 ColorInt32) { return static_cast<Uint8>((ColorInt32 & static_cast<Uint32>(0x00ff0000)) >> 16); };
    static constexpr Uint8 GetAFromInt32(Uint32 ColorInt32) { return static_cast<Uint8>((ColorInt32 & static_cast<Uint32>(0xff000000)) >> 24); };

    //static constexpr Uint32 RGBA2Int32(int r, int g, int b, int a) { return r + g * 256 + b * 256 * 256 + a * 256 * 256 * 256; };
    static constexpr Uint32 RGBA2Int32(int r, int g, int b, int a) { return static_cast<Uint32>(r) | static_cast<Uint32>(g) << 8 | static_cast<Uint32>(b) << 16 | static_cast<Uint32>(a) << 24; };
};

class BWrapper
{
public:
    // scoped enums:
    enum struct OS : unsigned char { iOS, Windows, AndroidOS, /*Mac,  Linux,*/ Unknown };
    enum struct KeyCodes : unsigned {
        Esc, BackSpace, Back, Enter, Tab, Delete, F1, Help, Home, End, Insert, Find, Copy, PageDown, PageUp, Paste, Pause, PrintScreen, Return, Return2, Space, Left, Right, Up, Down, Uknown, Minus, Plus, Equals, LeftBraket, RightBraket, Comma, Period, Quote,
        N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, // main numbers
        Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9, NumpadPlus, NumpadMinus,
        Q, W, E, R, T, Y, U, I, O, P, A, S, D, F, G, H, J, K, L, Z, X, C, V, B, N, M
    };
    enum struct FileSearchPriority : unsigned char { Assets, FileSystem };
    enum struct FileOpenMode : unsigned char { ReadBinary, WriteBinary, AppendBinary /*, WriteText, ReadText*/ }; // http://www.cplusplus.com/reference/cstdio/fopen/
    enum struct AndroidToastDuration : int { Short = 0, Long = 1 };
    enum struct LogPriority          /*withouttype*/ { Verbose = SDL_LOG_PRIORITY_VERBOSE, Debug = SDL_LOG_PRIORITY_DEBUG, Info = SDL_LOG_PRIORITY_INFO, Warning = SDL_LOG_PRIORITY_WARN, Error = SDL_LOG_PRIORITY_ERROR, Critical = SDL_LOG_PRIORITY_CRITICAL }; // https://wiki.libsdl.org/SDL_LogMessage

    // scoped enum - requieres bitwise or (|) operations
    struct AndroidToastGravity {
        enum : int
        {
            /*  Custom Constant  */
            DO_NOT_USE_SET_GRAVITY_METHOD = -1,
            /* Android Constants */
            AXIS_CLIP = 8,
            AXIS_PULL_AFTER = 4,
            AXIS_PULL_BEFORE = 2,
            AXIS_SPECIFIED = 1,
            AXIS_X_SHIFT = 0,
            AXIS_Y_SHIFT = 4,
            BOTTOM = 80,
            CENTER = 17,
            CENTER_HORIZONTAL = 1,
            CENTER_VERTICAL = 16,
            CLIP_HORIZONTAL = 8,
            CLIP_VERTICAL = 128,
            DISPLAY_CLIP_HORIZONTAL = 16777216,
            DISPLAY_CLIP_VERTICAL = 268435456,
            END = 8388613,
            FILL = 119,
            FILL_HORIZONTAL = 7,
            FILL_VERTICAL = 112,
            HORIZONTAL_GRAVITY_MASK = 7,
            LEFT = 3,
            NO_GRAVITY = 0,
            RELATIVE_HORIZONTAL_GRAVITY_MASK = 8388615,
            RELATIVE_LAYOUT_DIRECTION = 8388608,
            RIGHT = 5,
            START = 8388611,
            TOP = 48,
            VERTICAL_GRAVITY_MASK = 112
        };
    };

    static std::string         GetAppBuildDateTimeString() { return std::string(__DATE__) + " " + __TIME__; };
    static std::string         GetSDKVersionInfo();

    // System init-quit functions
    static bool                Init(Uint32 flags);
    static void                Quit();
    static int                 GetCPUCount() { return SDL_GetCPUCount(); };

    // Windows and Render functions
    static AkkordWindow* CreateRenderWindow(const char* Title, int X, int Y, int W, int H, Uint32 Flags);
    static AkkordRenderer* CreateRenderer(AkkordWindow* window, int index, Uint32 flags);
    static bool                SetActiveWindow(AkkordWindow* Window);
    static bool                SetActiveRenderer(AkkordRenderer* Renderer);
    static AkkordWindow* GetActiveWindow();
    static AkkordRenderer* GetActiveRenderer();
    static bool                ClearRenderer();
    static bool                RefreshRenderer();
    static bool                FlushRenderer();
    static bool                DestroyRenderer();
    static bool                DestroyWindow();
    static bool                SetWindowResizable(bool Resizable);
    static bool                SetWindowSize(int W, int H);
    static AkkordPoint         GetScreenSize();
    static int                 GetScreenWidth() { return GetScreenSize().x; };
    static int                 GetScreenHeight() { return GetScreenSize().y; };

    // Drawing functions
    static bool                SetCurrentColor(const AkkordColor& Color);
    static bool                DrawRect(const AkkordRect& Rect);
    static bool                DrawRect(int X, int Y, int W, int H);
    static bool                FillRect(const AkkordRect& Rect);
    static bool                FillRect(int X, int Y, int W, int H);
    static bool                DrawLine(const AkkordPoint& Point1, const AkkordPoint& Point2);
    static bool                DrawLine(int P1X, int P1Y, int P2X, int P2Y);
    static bool                IsPointInRect(const AkkordPoint& Point, const AkkordRect& Rect) { return (Rect.x <= Point.x && Point.x <= Rect.x + Rect.w) && (Rect.y <= Point.y && Point.y <= Rect.y + Rect.h); };
    static int                 GetDisplayDPI(int DisplayIndex, float* Ddpi, float* Hdpi, float* Vdpi);

    // Working with files
    static char* File2Buffer(const char* FileName, BWrapper::FileSearchPriority SearchPriority, unsigned& BufferSize);
    static FILE* FileOpen(const char* FileName, BWrapper::FileSearchPriority SearchPriority, BWrapper::FileOpenMode OpenMode);
    static bool                FileWrite(FILE* File, const void* Buffer, size_t Size, size_t Count);
    static bool                FilePutS(FILE* File, const char* String);
    static bool                FileWriteFormatted(FILE* File, const char* Format, ...);
    static bool                FileExists(const char* FileName, BWrapper::FileSearchPriority SearchPriority);
    static bool                FileDelete(const char* FileName);
    static bool                FileRename(const char* OldName, const char* NewName);
    static void                FileClose(FILE*& File);
    static void                CloseBuffer(char*& buffer);

    // Working with directories
    static bool                DirCreate(const char* Dir);
    static bool                DirExists(const char* Dir);
    static bool                DirRemoveRecursive(const char* Dir);
    static std::string         GetInternalDir();
    static std::string         GetInternalWriteDir();

    // Conversion functions
    static std::string         Int2Str(int Num);
    static unsigned            Str2Num(const char* Str);

    // Event-handling functions
    static BWrapper::KeyCodes  DecodeKey(const SDL_Keysym& SDL_Key);

    // Logging and debugging functions
    static void                Log(BWrapper::LogPriority Priority, const char* File, const char* Function, unsigned Line, SDL_PRINTF_FORMAT_STRING const char* Fmt, ...);
    static LogParamsStruct* GetLogParams();
    static void                SetLogPriority(BWrapper::LogPriority Priority) { SDL_LogPriority sev = (SDL_LogPriority)Priority; SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, sev); };
    static bool                PrintDirContent(const char* Path, BWrapper::LogPriority Priority = BWrapper::LogPriority::Debug, bool Recursive = false);

    // Random functions
    static bool                RandomInit(); // Starts random generator. It is possible call it multiple times.
    static int                 Random(); // Return random nonnegative value

    // Activity functions
    static bool                OpenURL(const char* url);

    // Android-specific functions
    static int                 AndroidGetApiLevel();
    static bool                AndroidShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity = BWrapper::AndroidToastGravity::DO_NOT_USE_SET_GRAVITY_METHOD, int xOffset = 0, int yOffset = 0);

    // Datetime functions
    static time_t              GetTimeSeconds(); // https://ru.wikipedia.org/wiki/Time.h
    static Uint32              GetTicks() { return SDL_GetTicks(); }; // Returns an unsigned 32 - bit value representing the number of milliseconds since the SDL library initialized

    // Thread functions
    static void                Sleep(unsigned MilliSeconds) { SDL_Delay(MilliSeconds); };

    // Environment functions
    static Locale::Lang        GetDeviceLanguage();
    static std::string         GetEnvVariable(const char* Variable); // Only for windows

    // Feature functions
    static void                ShareText(const char* Title, const char* Message);
    static void                SharePNG(const char* Title, const char* File);

    static int                 GetAudioOutputRate(); // only for Android
    static int                 GetAudioOutputBufferSize(); // only for Android

    static constexpr BWrapper::OS GetDeviceOS()
    {
#ifdef __APPLE__
        return BWrapper::OS::iOS;
#endif

#ifdef __ANDROID__
        return BWrapper::OS::AndroidOS;
#endif

#ifdef __WIN32__
        return  BWrapper::OS::Windows;
#endif

        return BWrapper::OS::Unknown;
    };

    static constexpr bool IsReleaseBuild()
    {
#ifdef __AKK0RD_DEBUG_MACRO__
        return false;
#endif
        return true;
    }

    //Запрещаем создавать экземпляр класса BWrapper
    BWrapper() = delete;
    ~BWrapper() = delete;
    BWrapper(const BWrapper& rhs) = delete; // Копирующий: конструктор
    BWrapper(BWrapper&& rhs) = delete; // Перемещающий: конструктор
    BWrapper& operator= (const BWrapper& rhs) = delete; // Оператор копирующего присваивания
    BWrapper& operator= (BWrapper&& rhs) = delete; // Оператор перемещающего присваивания
};

class AkkordTexture
{
private:
    SDL_Texture* tex = nullptr;
public:
    enum struct TextureType : unsigned char { BMP, PNG, JPEG, SVG };
    struct Flip { enum : unsigned char { None = SDL_FLIP_NONE, Horizontal = SDL_FLIP_HORIZONTAL, Vertical = SDL_FLIP_VERTICAL }; };
    void Destroy() { if (tex) { SDL_DestroyTexture(tex); }; tex = nullptr; };
    //bool LoadFromFile(const char* FileName);
    bool LoadFromFile(const char* FileName, TextureType Type, const BWrapper::FileSearchPriority SearchPriority = BWrapper::FileSearchPriority::Assets, float Scale = 1.0f);
    bool LoadFromMemory(const char* Buffer, int Size, TextureType Type, float Scale = 1.0f);
    bool CreateFromSurface(SDL_Surface* Surface);
    //int Draw(AkkordRect Rect);
    //int Draw(AkkordRect RectFromAtlas, AkkordRect Rect);
    bool Draw(const AkkordRect& Rect, const AkkordRect* RectFromAtlas = nullptr) const;
    bool Draw(const AkkordRect& Rect, const AkkordRect* RectFromAtlas, unsigned char Flip, double Angle, AkkordPoint* Point) const;
    AkkordPoint GetSize() const;
    bool SetColorMod(Uint8 R, Uint8 G, Uint8 B);
    bool SetColorMod(const AkkordColor& ModColor) { return SetColorMod(ModColor.GetR(), ModColor.GetG(), ModColor.GetB()); };
    bool SetAlphaMod(Uint8 A);

    AkkordTexture() : tex(nullptr) {};
    ~AkkordTexture() { Destroy(); };

    SDL_Texture* GetTexture() { return tex; };

    AkkordTexture(const AkkordTexture& rhs) = delete; // Копирующий: конструктор
    AkkordTexture(AkkordTexture&& tmp) { this->tex = tmp.tex; tmp.tex = nullptr; }; // Перемещающий конструктор объявлен
    AkkordTexture& operator= (const AkkordTexture& rhs) = delete; // Оператор копирующего присваивания
    AkkordTexture& operator= (AkkordTexture&& rhs) = delete; // Оператор перемещающего присваивания
};

class DirContentReader
{
private:
    DirContentElementArray List;
    unsigned Size = 0;
    unsigned Pointer = 0;
public:
    bool Open(const char* Dir);
    bool Close() { List.clear(); this->Size = this->Pointer = 0; return true; };
    bool Next(DirContentElement*& Element);
    DirContentReader() : Size(0), Pointer(0) {};
    ~DirContentReader() { Close(); };

    DirContentReader(const DirContentReader& rhs) = delete; // Копирующий: конструктор
    DirContentReader(DirContentReader&& rhs) = default; // Перемещающий: конструктор
    DirContentReader& operator= (const DirContentReader& rhs) = delete; // Оператор копирующего присваивания
    DirContentReader& operator= (DirContentReader&& rhs) = delete; // Оператор перемещающего присваивания
};

class FileReader
{
private:

#ifdef __ANDROID__ // На андроиде с assets придется работать особым способом
    char* buffer = nullptr;
    membuf* sbuf = nullptr;
#endif
    std::filebuf fb;
    std::istream* in = nullptr;
    bool opened = false;
public:
    bool Open(const char* Fname, BWrapper::FileSearchPriority SearchPriority);
    void Close();
    FileReader() { Close(); };
    ~FileReader() { Close(); };
    bool IsOpen() const { return opened; };
    bool ReadLine(std::string& Line);
    bool Read(char* Buffer, unsigned MaxSize, unsigned& Readed);

    FileReader(const FileReader& rhs) = delete; // Копирующий: конструктор
    FileReader(FileReader&& rhs) = delete; // Перемещающий: конструктор
    FileReader& operator= (const FileReader& rhs) = delete; // Оператор копирующего присваивания
    FileReader& operator= (FileReader&& rhs) = delete; // Оператор перемещающего присваивания
};

class msgBox
{
public:
    enum struct Action : int { Cancel = 0, Button1 = 1, Button2 = 2, Button3 = 3 };

    static void   Show(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2 = nullptr, const char* Button3 = nullptr, Uint32 TimeOutMS = 0);
    static Uint32 GetEventCode();
    static void   DecodeEvent(const SDL_Event& Event, int& Code, msgBox::Action& Action);

    //Запрещаем создавать экземпляр класса msgBox
    msgBox() = delete;
    ~msgBox() = delete;
    msgBox(const msgBox& rhs) = delete; // Копирующий: конструктор
    msgBox(msgBox&& rhs) = delete; // Перемещающий: конструктор
    msgBox& operator= (const msgBox& rhs) = delete; // Оператор копирующего присваивания
    msgBox& operator= (msgBox&& rhs) = delete; // Оператор перемещающего присваивания
};

class WAVPlayer
{
    Uint32 wav_length = 0;      // length of our sample
    Uint8* wav_buffer = nullptr;      // buffer containing our audio file
    SDL_AudioSpec wav_spec; // the specs of our piece of music
    SDL_AudioDeviceID deviceId = 0;

public:
    bool LoadFromFile(const char* FileName, const BWrapper::FileSearchPriority SearchPriority = BWrapper::FileSearchPriority::Assets);
    bool LoadFromMemory(const char* Buffer, int Size);
    bool Play();
    void Clear();

    WAVPlayer() :wav_length(0), wav_buffer(nullptr), deviceId(0) {};
    ~WAVPlayer() { Clear(); };

    WAVPlayer(const WAVPlayer& rhs) = delete; // Копирующий: конструктор
    WAVPlayer(WAVPlayer&& rhs) = delete; // Перемещающий: конструктор
    WAVPlayer& operator= (const WAVPlayer& rhs) = delete; // Оператор копирующего присваивания
    WAVPlayer& operator= (WAVPlayer&& rhs) = delete; // Оператор перемещающего присваивания
};

#endif // __AKK0RD_BASE_BASEWRAPPER_H__
