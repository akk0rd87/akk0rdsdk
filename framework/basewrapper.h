#pragma once
#ifndef __AKK0RD_BASE_BASEWRAPPER_H__
#define __AKK0RD_BASE_BASEWRAPPER_H__

#define _CRT_SECURE_NO_WARNINGS

#include "framework_defines.h"
//#include "userevents.h"
#include "core/locale.h"

class AkkordPoint 
{ 
    public:
    int x, y;
    AkkordPoint();
    AkkordPoint(int X, int Y);
};

class AkkordRect
{
public:
    int x, y, w, h;
    AkkordRect();    
    AkkordRect(int X, int Y, int W, int H);
    AkkordRect(AkkordPoint Point1, AkkordPoint Point2);    
};

class AkkordColor
{
private :
    unsigned int color;
public:
    AkkordColor();
    AkkordColor(unsigned int Color);
    AkkordColor(unsigned char R, unsigned char G, unsigned char B);
    AkkordColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A);

    void                 SetInt32(unsigned int Color);
    void                 SetRGB  (unsigned char R, unsigned char G, unsigned char B);
    void                 SetRGBA (unsigned char R, unsigned char G, unsigned char B, unsigned char A);
    
    void                 SetR    (unsigned char R);
    void                 SetG    (unsigned char G);
    void                 SetB    (unsigned char B);
    void                 SetA    (unsigned char A);
    
    const unsigned int   GetInt32();    
    const unsigned char  GetR    ();
    const unsigned char  GetG    ();
    const unsigned char  GetB    ();
    const unsigned char  GetA    ();

    static unsigned char GetRFromInt32(unsigned int ColorInt32);
    static unsigned char GetGFromInt32(unsigned int ColorInt32);
    static unsigned char GetBFromInt32(unsigned int ColorInt32);
    static unsigned char GetAFromInt32(unsigned int ColorInt32);

    static unsigned int RGBA2Int32(int r, int g, int b, int a);
};

class BWrapper
{   
public:
    // scoped enums:
    enum struct OS                   : unsigned char { iOS, Windows, AndroidOS, /*Mac,  Linux,*/ Unknown };
    
    //enum struct Lang                 : unsigned      { Russian, Ukrainian, Armenian, Belarussian, Uzbek, Kazakh, Azerbaijani, English, Chinese, French, Japanese, Bulgarian, Unknown };
    
    enum struct KeyCodes             : unsigned      { Esc, BackSpace, Back, Enter, Tab, Delete, F1, Help, Home, End, Insert, Find, Copy, PageDown, PageUp, Paste, Pause, PrintScreen, Return, Return2, Space, Left, Right, Up, Down, Uknown, Minus, Plus, Equals,
													  N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, // main numbers
													  Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,// numpad
													  Q,W,E,R,T,Y,U,I,O,P,A,S,D,F,G,H,J,K,L,Z,X,C,V,B,N,M}; 
    enum struct FileSearchPriority   : unsigned char { Assets, FileSystem };
    enum struct FileOpenMode         : unsigned char { ReadBinary, WriteBinary, AppendBinary /*, WriteText, ReadText*/ }; // http://www.cplusplus.com/reference/cstdio/fopen/
    enum struct AndroidToastDuration : int           { Short = 0, Long = 1 };        
    enum struct LogPriority          /*withouttype*/ { Verbose = SDL_LOG_PRIORITY_VERBOSE, Debug = SDL_LOG_PRIORITY_DEBUG, Info = SDL_LOG_PRIORITY_INFO, Warning = SDL_LOG_PRIORITY_WARN, Error = SDL_LOG_PRIORITY_ERROR, Critical = SDL_LOG_PRIORITY_CRITICAL }; // https://wiki.libsdl.org/SDL_LogMessage
    

    // scoped enum - requieres bitwise or (|) operations
    struct AndroidToastGravity{
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

    
    static std::string         GetSDKVersionInfo           ();
                                                           
    // System init-quit functions                          
    static bool                Init                        (Uint32 flags);
    static void                Quit                        ();

    // Windows and Render functions
    static AkkordWindow*       CreateRenderWindow          (const char* Title, int X, int Y, int W, int H, Uint32 Flags);
    static AkkordRenderer*     CreateRenderer              (AkkordWindow* window, int index, Uint32 flags);
    static bool                SetActiveWindow             (AkkordWindow* Window);
    static bool                SetActiveRenderer           (AkkordRenderer* Renderer);
    static AkkordWindow*       GetActiveWindow             ();
    static AkkordRenderer*     GetActiveRenderer           ();
    static bool                ClearRenderer               ();
    static bool                RefreshRenderer             ();        
    static bool                DestroyRenderer             ();
    static bool                DestroyWindow               ();
    static bool                SetWindowResizable          (bool Resizable);
    static bool                SetWindowSize               (int W, int H);
    static AkkordPoint         GetScreenSize               ();
    static int                 GetScreenWidth              ();
    static int                 GetScreenHeight             ();    
                                                           
                                                           
    // Drawing functions                                      
    static bool                SetCurrentColor             (AkkordColor Color);
    static bool                DrawRect                    (AkkordRect Rect);
    static bool                DrawRect                    (int X, int Y, int W, int H);
    static bool                FillRect                    (AkkordRect Rect);
    static bool                FillRect                    (int X, int Y, int W, int H);
    static bool                DrawLine                    (AkkordPoint Point1, AkkordPoint Point2);
    static bool                IsPointInRect               (AkkordPoint Point, AkkordRect Rect);    
                                                           
                                                           
    // Environment functions                               
    static Locale::Lang        GetDeviceLanguage();        
    static BWrapper::OS        GetDeviceOS                 ();
    static std::string         GetEnvVariable              (const char* Variable); // Only for windows
                                                           
                                                           
    // Working with files                                  
    static char*               File2Buffer                 (const char* FileName, BWrapper::FileSearchPriority SearchPriority, unsigned& BufferSize);
    static FILE*               FileOpen                    (const char* FileName, BWrapper::FileSearchPriority SearchPriority, BWrapper::FileOpenMode OpenMode);
    static bool                FileWrite                   (FILE* File, const void* Buffer, size_t Size, size_t Count);
    static bool                FilePutS                    (FILE* File, const char * String);
    static bool                FileWriteFormatted          (FILE* File, const char * Format, ...);
    static bool                FileExists                  (const char* FileName, BWrapper::FileSearchPriority SearchPriority);
    static bool                FileDelete                  (const char* FileName);
    static bool                FileRename                  (const char* OldName, const char* NewName);
    static void                FileClose                   (FILE*& File);
    static void                CloseBuffer                 (char*& buffer);                                                           
                                                           
    // Working with directories                            
    static bool                DirCreate                   (const char* Dir);
    static bool                DirExists                   (const char* Dir);    
    static bool                DirRemoveRecursive          (const char* Dir);
    static std::string         GetInternalDir              ();
    static std::string         GetInternalWriteDir         ();
                                                           
    // Conversion functions                                
    static std::string         Int2Str                     (int Num);
    static unsigned            Str2Num                     (const char* Str);
                                                           
    // Event-handling functions                            
    static BWrapper::KeyCodes  DecodeKey                   (SDL_Keysym SDL_Key);

        // MessageBox functions    
    static int                 ShowMessageBox              (const char* Message);
    //static void                MessageBoxSetColorScheme    (MessageBoxColorScheme& Scheme);
    //static int                 MessageBoxShow              (BWrapper::MessageBoxPriority Priority, const char* Title, const char* Message, AkkordWindow* ParentWindow, const char* Button0, const char* Button1 = nullptr, const char* Button2 = nullptr);
                                                           
    // Logging and debugging functions                     
    static void                Log                         (BWrapper::LogPriority Priority, const char* File, const char* Function, unsigned Line, SDL_PRINTF_FORMAT_STRING const char *Fmt, ...); 
    static LogParamsStruct*    GetLogParams                ();
    static void                SetLogPriority              (BWrapper::LogPriority Priority);
    static bool                PrintDirContent             (const char* Path, BWrapper::LogPriority Priority = BWrapper::LogPriority::Debug, bool Recursive = false);
                                                                                                                      
    // Random functions                                    
    static bool                RandomInit                  (); // Starts random generator. It is possible call it multiple times.
    static int                 Random                      (); // Return random nonnegative value
                                                           
                                                           
    // Activity functions                                  
    static bool                OpenURL                     (const char* url);
                                                           
                                                           
    // Android-specific functions                          
    static int                 AndroidGetApiLevel          ();
    static bool                AndroidShowToast            (const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity = BWrapper::AndroidToastGravity::DO_NOT_USE_SET_GRAVITY_METHOD, int xOffset = 0, int yOffset = 0);
                                                           
                                                           
    // Datetime functions                                  
    static time_t              GetTimeSeconds              (); // https://ru.wikipedia.org/wiki/Time.h
    static unsigned            GetTicks                    (); // Returns an unsigned 32 - bit value representing the number of milliseconds since the SDL library initialized
                                                           
                                                           
    // Thread functions                                    
    static void                Sleep                       (unsigned MilliSeconds);
    


    // For test. Do not use this function in your projects
    //static void                Test                ();



    // Not realized
    //static int GetDisplayDPI(float* Ddpi, float* Hdpi, float* Vdpi);
};

class AkkordTexture
{
private:   
    SDL_Texture *tex = nullptr;
public:
    enum struct TextureType : unsigned char { BMP, PNG, JPEG };
    struct Flip { enum : unsigned char { None = SDL_FLIP_NONE, Horizontal = SDL_FLIP_HORIZONTAL, Vertical = SDL_FLIP_VERTICAL }; };  
    void Destroy();
    //bool LoadFromFile(const char* FileName);
    bool LoadFromFile(const char* FileName, TextureType Type, const BWrapper::FileSearchPriority SearchPriority = BWrapper::FileSearchPriority::Assets);
    //int Draw(AkkordRect Rect);
    //int Draw(AkkordRect RectFromAtlas, AkkordRect Rect);
    const bool Draw(AkkordRect Rect, const AkkordRect* RectFromAtlas = nullptr, unsigned char Flip = AkkordTexture::Flip::None, double Angle = 0, AkkordPoint* Point = nullptr);
    const AkkordPoint GetSize();
    AkkordTexture();
    ~AkkordTexture();
};

class DirContentReader
{
private:    
    DirContentElementArray List;
    unsigned Size    = 0;
    unsigned Pointer = 0;    
public:
    bool Open(const char* Dir);
    bool Close();
    bool Next(DirContentElement*& Element);    
    ~DirContentReader();
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
    bool Open(const char *Fname, BWrapper::FileSearchPriority SearchPriority);
    void Close();
    FileReader();
    ~FileReader();
    const bool IsOpen();
    bool ReadLine(std::string& Line);
    bool Read(char* Buffer, unsigned MaxSize, unsigned& Readed);
};


class MessageBox
{
private:
    struct MessageBoxColor{ unsigned char r, g, b; };
    MessageBoxColor BackGround, Text, ButtonBorder, ButtonBackGround, ButtonSelected;
    AkkordWindow* ParentWindow = nullptr;
public:
    enum struct Priority  /*withouttype*/ { Info = SDL_MESSAGEBOX_INFORMATION, Warning = SDL_MESSAGEBOX_WARNING, Error = SDL_MESSAGEBOX_ERROR };
private:    
public:

    void SetBackGroundColor(unsigned char R, unsigned char G, unsigned char B) { BackGround.r = R; BackGround.g = G; BackGround.b = B; };
    void SetTextColor(unsigned char R, unsigned char G, unsigned char B) { ButtonBorder.r = R; ButtonBorder.g = G; ButtonBorder.b = B; };
    void SetButtonBorderColor(unsigned char R, unsigned char G, unsigned char B) { ButtonBorder.r = R; ButtonBorder.g = G; ButtonBorder.b = B; };
    void SetButtonBackGroundColor(unsigned char R, unsigned char G, unsigned char B) { ButtonBackGround.r = R; ButtonBackGround.g = G; ButtonBackGround.b = B; };
    void SetButtonSelectedColor(unsigned char R, unsigned char G, unsigned char B) { ButtonSelected.r = R; ButtonSelected.g = G; ButtonSelected.b = B; };
    void SetParentWindow(AkkordWindow* ParentWindow){ this->ParentWindow = ParentWindow; }

	int Show(MessageBox::Priority MessageBoxPriority, const char* Title, const char* Message, const char* Button0, const char* Button1 = nullptr, const char* Button2 = nullptr)
    {
        const SDL_MessageBoxColorScheme SDLColorScheme =
        {
            { /* .colors (.r, .g, .b) */
                /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
                { BackGround.r, BackGround.g, BackGround.b },
                /* [SDL_MESSAGEBOX_COLOR_TEXT] */
                { Text.r, Text.g, Text.b },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
                { ButtonBorder.r, ButtonBorder.g, ButtonBorder.b },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
                { ButtonBackGround.r, ButtonBackGround.g, ButtonBackGround.b },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
                { ButtonSelected.r, ButtonSelected.g, ButtonSelected.b }
            }
        };

        SDL_MessageBoxButtonData Buttons[3];

        int Count = 1 + (Button1 ? 1 : 0) + (Button2 ? 1 : 0);
        int ButtonCnt = 0;

        if (Button2)
        {
            Buttons[ButtonCnt].text = Button2;
            Buttons[ButtonCnt].flags = 0; //SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
            Buttons[ButtonCnt].buttonid = Count - ButtonCnt - 1;
            ++ButtonCnt;
        }

        if (Button1)
        {
            Buttons[ButtonCnt].text = Button1;
            Buttons[ButtonCnt].flags = 0; //= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
            Buttons[ButtonCnt].buttonid = Count - ButtonCnt - 1;
            ++ButtonCnt;
        }

        Buttons[ButtonCnt].text = Button0;
        Buttons[ButtonCnt].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
        Buttons[ButtonCnt].buttonid = Count - ButtonCnt - 1;
        ++ButtonCnt;

        const SDL_MessageBoxData messageboxdata = {
            (SDL_MessageBoxFlags)MessageBoxPriority, /* .flags */
            ParentWindow, /* .window */
            Title, /* .title */
            Message, /* .message */
            ButtonCnt, /*SDL_arraysize(buttons),*/ /* .numbuttons */
            Buttons, /* .buttons */
            &SDLColorScheme /* .colorScheme */
        };

        int buttonid;

        if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0)
        {
            logError("MessageBox show error");
            return -2;
        }

        if (buttonid == -1) {
            logVerbose("No selection");
            return -1;
        }

        return buttonid;
    };
};

#include "customevents.h"

#endif // __AKK0RD_BASE_BASEWRAPPER_H__
