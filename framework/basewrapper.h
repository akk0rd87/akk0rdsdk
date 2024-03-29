#ifndef __AKK0RD_SDK_BASE_BASEWRAPPER_H__
#define __AKK0RD_SDK_BASE_BASEWRAPPER_H__

#include "framework_defines.h"
#include "core/locale.h"

class AkkordPoint : public SDL_Point
{
public:
    constexpr AkkordPoint() : SDL_Point() { x = y = 0; };
    constexpr AkkordPoint(int X, int Y) : SDL_Point() { x = X;  y = Y; };
    constexpr AkkordPoint(const AkkordPoint& Point) : SDL_Point() { x = Point.x; y = Point.y; };
    bool operator== (const AkkordPoint& Point) const { return Point.x == x && Point.y == y; };
    bool operator!= (const AkkordPoint& Point) const { return !(Point == *this); };

    AkkordPoint& SetX(int X) { x = X; return *this; }
    AkkordPoint& SetY(int Y) { y = Y; return *this; }

    int GetX() const { return x; }
    int GetY() const { return y; }
};

class AkkordFPoint : public SDL_FPoint
{
public:
    constexpr AkkordFPoint() : SDL_FPoint() { x = y = 0.0F; };
    constexpr AkkordFPoint(float X, float Y) : SDL_FPoint() { x = X;  y = Y; };
    constexpr AkkordFPoint(const AkkordFPoint& Point) : SDL_FPoint() { x = Point.x; y = Point.y; };

    AkkordFPoint& SetX(float X) { x = X; return *this; }
    AkkordFPoint& SetY(float Y) { y = Y; return *this; }

    float GetX() const { return x; }
    float GetY() const { return y; }
};

class AkkordRect : public SDL_Rect
{
public:
    constexpr AkkordRect() : SDL_Rect() { x = y = w = h = 0; };
    constexpr AkkordRect(const AkkordRect& Rect) : SDL_Rect() { x = Rect.x;  y = Rect.y; w = Rect.w; h = Rect.h; };
    constexpr AkkordRect(int X, int Y, int W, int H) : SDL_Rect() { x = X; y = Y; w = W; h = H; };
    constexpr AkkordRect(const AkkordPoint& Point1, const AkkordPoint& Point2) : SDL_Rect() { x = Point1.x; y = Point1.y;  w = Point2.x; h = Point2.y; };

    AkkordRect& SetW(int W) { w = W; return *this; };
    AkkordRect& SetH(int H) { h = H; return *this; };
    AkkordRect& SetX(int X) { x = X; return *this; };
    AkkordRect& SetY(int Y) { y = Y; return *this; };

    int GetW() const { return w; };
    int GetH() const { return h; };
    int GetX() const { return x; };
    int GetY() const { return y; };

    AkkordRect& SetPosition(const AkkordPoint& Position) { x = Position.GetX(); y = Position.GetY(); return *this; };
    AkkordRect& SetPosition(int X, int Y) { x = X; y = Y; return *this; };
    AkkordRect& SetSize(const AkkordPoint& Size) { w = Size.GetX(); h = Size.GetY(); return *this; };
    AkkordRect& SetSize(int W, int H) { w = W; h = H; return *this; };

    AkkordPoint GetPosition() const { return AkkordPoint(x, y); }
    AkkordPoint GetSize() const { return AkkordPoint(w, h); }

    bool operator== (const AkkordRect& Rect) const { return Rect.x == x && Rect.y == y && Rect.w == w && Rect.h == h; };
    bool operator!= (const AkkordRect& Rect) const { return !(Rect == *this); };
};

class AkkordFRect : public SDL_FRect
{
public:
    constexpr AkkordFRect() : SDL_FRect() { x = y = w = h = 0.0F; };
    constexpr AkkordFRect(const AkkordFRect& Rect) : SDL_FRect() { x = Rect.x;  y = Rect.y; w = Rect.w; h = Rect.h; };
    constexpr AkkordFRect(float X, float Y, float W, float H) : SDL_FRect() { x = X; y = Y; w = W; h = H; };
    // constexpr AkkordFRect(const AkkordPoint& Point1, const AkkordPoint& Point2) : SDL_FRect() { x = Point1.x; y = Point1.y;  w = Point2.x; h = Point2.y; };

    AkkordFRect& SetW(float W) { w = W; return *this; };
    AkkordFRect& SetH(float H) { h = H; return *this; };
    AkkordFRect& SetX(float X) { x = X; return *this; };
    AkkordFRect& SetY(float Y) { y = Y; return *this; };

    float GetW() const { return w; };
    float GetH() const { return h; };
    float GetX() const { return x; };
    float GetY() const { return y; };

    //AkkordRect& SetPosition(const AkkordPoint& Position) { x = Position.GetX(); y = Position.GetY(); return *this; };
    //AkkordRect& SetPosition(float X, float Y) { x = X; y = Y; return *this; };
    //AkkordRect& SetSize(const AkkordPoint& Size) { w = Size.GetX(); h = Size.GetY(); return *this; };
    //AkkordRect& SetSize(float W, float H) { w = W; h = H; return *this; };

    //AkkordPoint GetPosition() const { return AkkordPoint(x, y); }
    //AkkordPoint GetSize() const { return AkkordPoint(w, h); }
};

class AkkordColor
{
private:
    Uint32 ABGRcolor;
public:
    constexpr AkkordColor() : ABGRcolor{ 0 } {};
    constexpr explicit AkkordColor(Uint32 Color) : ABGRcolor(Color) {};
    constexpr AkkordColor(Uint8 R, Uint8 G, Uint8 B, Uint8 A) : ABGRcolor{ RGBA2Int32(R, G, B, A) } {};
    constexpr AkkordColor(Uint8 R, Uint8 G, Uint8 B) : AkkordColor(R, G, B, 255) {};

    AkkordColor& SetUint32(Uint32 ARGB) { ABGRcolor = ARGB; return *this; };
    AkkordColor& SetRGB(Uint8 R, Uint8 G, Uint8 B) { SetRGBA(R, G, B, static_cast<Uint8>(255)); return *this; };
    AkkordColor& SetRGBA(Uint8 R, Uint8 G, Uint8 B, Uint8 A) { ABGRcolor = AkkordColor::RGBA2Int32(R, G, B, A); return *this; };

    AkkordColor& SetR(Uint8 R) { ABGRcolor &= static_cast<Uint32>(0xffffff00); ABGRcolor |= static_cast<Uint32>(R); return *this; };
    AkkordColor& SetG(Uint8 G) { ABGRcolor &= static_cast<Uint32>(0xffff00ff); ABGRcolor |= (static_cast<Uint32>(G) << 8); return *this; };
    AkkordColor& SetB(Uint8 B) { ABGRcolor &= static_cast<Uint32>(0xff00ffff); ABGRcolor |= (static_cast<Uint32>(B) << 16); return *this; };
    AkkordColor& SetA(Uint8 A) { ABGRcolor &= static_cast<Uint32>(0x00ffffff); ABGRcolor |= (static_cast<Uint32>(A) << 24);  return *this; };

    // проставление цвета копипастом из Adobe Ullistrator
    void                 SetAiRGB24(Uint32 RGB) { SetRGBA(static_cast<Uint8>((RGB & static_cast<Uint32>(0xff0000)) >> 16), static_cast<Uint8>((RGB & static_cast<Uint32>(0x00ff00)) >> 8), static_cast<Uint8>((RGB & static_cast<Uint32>(0x0000ff))), 255); };

    Uint32               GetInt32() const { return ABGRcolor; };
    Uint8                GetR() const { return AkkordColor::GetRFromInt32(ABGRcolor); };
    Uint8                GetG() const { return AkkordColor::GetGFromInt32(ABGRcolor); };
    Uint8                GetB() const { return AkkordColor::GetBFromInt32(ABGRcolor); };
    Uint8                GetA() const { return AkkordColor::GetAFromInt32(ABGRcolor); };

    static constexpr Uint8 GetRFromInt32(Uint32 ColorInt32) { return static_cast<Uint8>((ColorInt32 & static_cast<Uint32>(0x000000ff))); };
    static constexpr Uint8 GetGFromInt32(Uint32 ColorInt32) { return static_cast<Uint8>((ColorInt32 & static_cast<Uint32>(0x0000ff00)) >> 8); };
    static constexpr Uint8 GetBFromInt32(Uint32 ColorInt32) { return static_cast<Uint8>((ColorInt32 & static_cast<Uint32>(0x00ff0000)) >> 16); };
    static constexpr Uint8 GetAFromInt32(Uint32 ColorInt32) { return static_cast<Uint8>((ColorInt32 & static_cast<Uint32>(0xff000000)) >> 24); };

    static constexpr Uint32 RGBA2Int32(int r, int g, int b, int a) { return static_cast<Uint32>(r) | (static_cast<Uint32>(g) << 8) | (static_cast<Uint32>(b) << 16) | (static_cast<Uint32>(a) << 24); };

    bool operator== (const AkkordColor& Color) const { return Color.ABGRcolor == ABGRcolor; };
    bool operator!= (const AkkordColor& Color) const { return Color.ABGRcolor != ABGRcolor; };
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
    static bool                SetCurrentColor(Uint8 R, Uint8 G, Uint8 B) { return SetCurrentColor(AkkordColor(R, G, B)); };
    static bool                SetCurrentColor(Uint8 R, Uint8 G, Uint8 B, Uint8 A) { return SetCurrentColor(AkkordColor(R, G, B, A)); };
    static bool                DrawRect(const AkkordRect& Rect);
    static bool                DrawRect(int X, int Y, int W, int H) { return DrawRect(AkkordRect(X, Y, W, H)); };
    static bool                DrawFRect(const AkkordFRect& Rect);
    static bool                DrawFRect(float X, float Y, float W, float H) { return DrawFRect(AkkordFRect(X, Y, W, H)); };
    static bool                FillRect(const AkkordRect& Rect);
    static bool                FillRect(int X, int Y, int W, int H) { return FillRect(AkkordRect(X, Y, W, H)); };
    static bool                FillFRect(const AkkordFRect& Rect);
    static bool                FillFRect(float X, float Y, float W, float H) { return FillFRect(AkkordFRect(X, Y, W, H)); };
    static bool                DrawRect(const AkkordFRect& Rect);
    static bool                DrawRect(float X, float Y, float W, float H) { return DrawRect(AkkordFRect(X, Y, W, H)); };
    static bool                FillRect(const AkkordFRect& Rect);
    static bool                FillRect(float X, float Y, float W, float H) { return FillRect(AkkordFRect(X, Y, W, H)); };
    static bool                DrawLine(const AkkordPoint& Point1, const AkkordPoint& Point2);
    static bool                DrawLine(int P1X, int P1Y, int P2X, int P2Y) { return DrawLine(AkkordPoint(P1X, P1Y), AkkordPoint(P2X, P2Y)); }
    static bool                DrawFLine(const AkkordFPoint& Point1, const AkkordFPoint& Point2);
    static bool                DrawFLine(float P1X, float P1Y, float P2X, float P2Y) { return DrawFLine(AkkordFPoint(P1X, P1Y), AkkordFPoint(P2X, P2Y)); };
    static bool                IsPointInRect(const AkkordPoint& Point, const AkkordRect& Rect) { return (Rect.x <= Point.x && Point.x <= Rect.x + Rect.w) && (Rect.y <= Point.y && Point.y <= Rect.y + Rect.h); };
    static int                 GetDisplayDPI(int DisplayIndex, float* Ddpi, float* Hdpi, float* Vdpi);

    // Working with files
    static FILE* FileOpen(const char* FileName, BWrapper::FileSearchPriority SearchPriority, BWrapper::FileOpenMode OpenMode);
    static bool                FileWrite(FILE* File, const void* Buffer, size_t Size, size_t Count);
    static bool                FilePutS(FILE* File, const char* String);
    static bool                FileWriteFormatted(FILE* File, const char* Format, ...);
    static bool                FileExists(const char* FileName, BWrapper::FileSearchPriority SearchPriority);
    static bool                FileDelete(const char* FileName);
    static bool                FileRename(const char* OldName, const char* NewName);
    static void                FileClose(FILE*& File);
    static std::unique_ptr<std::istream> GetAssetStream(const char* Fname);

    // Working with directories
    static bool                DirCreate(const char* Dir);
    static bool                DirExists(const char* Dir);
    static bool                DirRemoveRecursive(const char* Dir);
    //static std::string         GetInternalDir();
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
    static void                SharePDF(const char* Title, const char* File);

    static int                 GetAudioOutputRate(); // only for Android
    static int                 GetAudioOutputBufferSize(); // only for Android
    static bool                LaunchAppReviewIfAvailable(); // only for Android
    static bool                RequestFlexibleUpdateIfAvailable(); // only for Android

    static std::string         GetAppVersionName();
    static std::string         GetAppVersionCode();

    static constexpr BWrapper::OS GetDeviceOS()
    {
#ifdef __APPLE__
        return BWrapper::OS::iOS;
#endif

#ifdef __ANDROID__
        return BWrapper::OS::AndroidOS;
#endif

#ifdef __WINDOWS__
        return  BWrapper::OS::Windows;
#endif

        return BWrapper::OS::Unknown;
    };

    static constexpr bool IsReleaseBuild()
    {
#ifdef __AKK0RD_SDK_DEBUG_MACRO__
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
    std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)> tex;
public:
    enum struct TextureType : Uint8 { BMP, PNG, JPEG, SVG };
    enum struct Flip : Uint8 { None = SDL_FLIP_NONE, Horizontal = SDL_FLIP_HORIZONTAL, Vertical = SDL_FLIP_VERTICAL };
    void Destroy() { tex.reset(); };
    bool LoadFromFile(const char* FileName, TextureType Type, const BWrapper::FileSearchPriority SearchPriority = BWrapper::FileSearchPriority::Assets, float Scale = 1.0f);
    bool LoadFromMemory(const char* Buffer, int Size, TextureType Type, float Scale = 1.0f);
    bool CreateFromSurface(SDL_Surface* Surface);
    bool Draw(const AkkordRect& Rect, const AkkordRect* RectFromAtlas = nullptr) const;
    bool Draw(const AkkordRect& Rect, const AkkordRect* RectFromAtlas, AkkordTexture::Flip Flip, double Angle, AkkordPoint* Point) const;
    AkkordPoint GetSize() const;
    bool SetColorMod(Uint8 R, Uint8 G, Uint8 B);
    bool SetColorMod(const AkkordColor& ModColor) { return SetColorMod(ModColor.GetR(), ModColor.GetG(), ModColor.GetB()); };
    bool SetAlphaMod(Uint8 A);
    SDL_Texture* GetTexture() { return tex.get(); };

    AkkordTexture() : tex(nullptr, nullptr) {};

    AkkordTexture(const AkkordTexture& rhs) = delete; // Копирующий: конструктор
    AkkordTexture& operator= (const AkkordTexture& rhs) = delete; // Оператор копирующего присваивания
    AkkordTexture(AkkordTexture&& rhs) = default; // Перемещающий: конструктор
    AkkordTexture& operator= (AkkordTexture&& rhs) = default; // Оператор перемещающего присваивания
};

inline AkkordTexture::Flip operator | (AkkordTexture::Flip a, AkkordTexture::Flip b) {
    return static_cast<AkkordTexture::Flip>(static_cast<Uint8>(a) | static_cast<Uint8>(b));
}

inline AkkordTexture::Flip operator & (AkkordTexture::Flip a, AkkordTexture::Flip b) {
    return static_cast<AkkordTexture::Flip>(static_cast<Uint8>(a) & static_cast<Uint8>(b));
}

inline AkkordTexture::Flip operator |= (AkkordTexture::Flip a, AkkordTexture::Flip b) {
    a = a | b;
    return a;
}

inline bool operator!(AkkordTexture::Flip a) {
    return (static_cast<Uint8>(a) == 0);
}

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

    ~DirContentReader() = default;
    DirContentReader(const DirContentReader& rhs) = delete; // Копирующий: конструктор
    DirContentReader& operator= (const DirContentReader& rhs) = delete; // Оператор копирующего присваивания
    DirContentReader(DirContentReader&& rhs) = default; // Перемещающий: конструктор
    DirContentReader& operator= (DirContentReader&& rhs) = default; // Оператор перемещающего присваивания
};

class FileReader
{
private:
    std::unique_ptr<std::istream> in = nullptr;
    bool opened = false;
public:
    bool Open(const char* Fname, BWrapper::FileSearchPriority SearchPriority);
    void Close();
    FileReader() { Close(); };
    ~FileReader() { Close(); };
    bool IsOpen() const { return opened; };
    bool ReadLine(std::string& Line);
    bool Read(char* Buffer, unsigned MaxSize, unsigned& Readed);
    std::istream& GetInputStream() { return *(in.get()); }

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

#endif // __AKK0RD_SDK_BASE_BASEWRAPPER_H__
