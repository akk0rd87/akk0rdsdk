#pragma once
#ifndef __AKK0RD_PLATFORMS_H__
#define __AKK0RD_PLATFORMS_H__

#include "locale.h"

#ifdef __WIN32__
#include "windows/windows_wrapper.h"
#include <windows.h>
#endif

#ifdef __ANDROID__
#include "android/android_wrapper.h"
#endif

#ifdef __APPLE__
#include "ios/ios_wrapper.h"
#endif

struct InteralDirsStruct
{
public:
    bool Initialized = false;
    std::string InternalDir;
    std::string InternalWriteDir;
};

static InteralDirsStruct InteralDirs;

/////////////////////////////////////////
// Статический класс Platforms
/////////////////////////////////////////

class Platforms
{
private:
    static bool InitInternalDirs();
public:
    static bool                     Init();
    static Locale::Lang             GetDeviceLanguage();
    //static constexpr BWrapper::OS   GetDeviceOS();
    static std::string              GetEnvVariable   (const char* Variable); // Only for windows

    static std::string              GetInternalDir();
    static std::string              GetInternalWriteDir();
    static std::string              GetInternalAssetsDir();

    static bool                     DirCreate(const char* Dir);
    static bool                     DirExists(const char* Dir);
    static bool                     DirRemove(const char* Dir);
    static bool                     DirRemoveRecursive(const char* Dir);

    // Activity functions
    static bool                     OpenURL(const char* url);

    // Android-specific functions
    static int                      AndroidGetApiLevel();
    static bool                     AndroidShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset);

    static bool                     GetDirContent  (const char* Dir, DirContentElementArray& ArrayList);

    static void                     MessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, Uint32 TimeOutMS);
    static void                     ShareText(const char* Title, const char* Message);
};

/////////////////////////////////////////
// Статический класс Platforms
/////////////////////////////////////////

bool Platforms::InitInternalDirs()
{
    bool result = false;
#ifdef __ANDROID__
    AndroidWrapper::GetInternalDirs(InteralDirs.InternalDir, InteralDirs.InternalWriteDir);
    InteralDirs.Initialized = true;
    result = true;
#endif

#ifdef __APPLE__
    iOSWrapper::GetInternalWriteDir(InteralDirs.InternalWriteDir);
    InteralDirs.Initialized = true;
    result = true;
#endif

#ifdef __WIN32__
    InteralDirs.InternalWriteDir = std::string("data-ram/");
    InteralDirs.Initialized = true;
    result = true;
#endif

    if (result)
    {
        auto len = InteralDirs.InternalDir.length();
        if (len > 0)
            if (InteralDirs.InternalDir[len - 1] != '/' && InteralDirs.InternalDir[len - 1] != '\\')
                InteralDirs.InternalDir = InteralDirs.InternalDir + "/";

        len = InteralDirs.InternalWriteDir.length();
        if (len > 0)
            if (InteralDirs.InternalWriteDir[len - 1] != '/' && InteralDirs.InternalWriteDir[len - 1] != '\\')
                InteralDirs.InternalWriteDir = InteralDirs.InternalWriteDir + "/";

        //logDebug("Internal Write Dir %s", InteralDirs.InternalWriteDir.c_str());
    }

    return result;
};

std::string Platforms::GetInternalDir()
{
    if (!InteralDirs.Initialized) InitInternalDirs();
    return InteralDirs.InternalDir;
};

std::string Platforms::GetInternalWriteDir()
{
    if (!InteralDirs.Initialized) InitInternalDirs();
    return InteralDirs.InternalWriteDir;
};

std::string Platforms::GetInternalAssetsDir()
{
#ifdef __ANDROID__
    return "";
#endif
    return "assets/";
}

Locale::Lang Platforms::GetDeviceLanguage()
{
    std::string lang;
#if __ANDROID__
    lang = AndroidWrapper::GetLanguage();
#endif

#ifdef __WIN32__
    //lang = WindowsWrapper::GetLanguage();
    //return Locale::DecodeLang_ISO639_Code(lang.c_str());
    //return Locale::DecodeLang_ISO639_Code("+-");
#endif

#ifdef __APPLE__
    lang = iOSWrapper::GetLanguage();
#endif

    return Locale::DecodeLang_ISO639_Code(lang.c_str());
};


bool Platforms::DirCreate(const char* Dir)
{
#ifdef __WIN32__
    return WindowsWrapper::DirCreate(Dir);
#endif

#ifdef __ANDROID__
    return AndroidWrapper::DirCreate(Dir);
#endif

#ifdef __APPLE__
    return iOSWrapper::DirCreate(Dir);
#endif

    return false;
}

bool Platforms::OpenURL(const char* url)
{
#ifdef __ANDROID__
    return AndroidWrapper::OpenURL(url);
#endif // __ANDROID__

#ifdef __WIN32__
    return WindowsWrapper::OpenURL(url);
#endif // __WIN32__

#ifdef __APPLE__
    return iOSWrapper::OpenURL(url);
#endif // __APPLE__

    return false;
}

/*
constexpr BWrapper::OS Platforms::GetDeviceOS()
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
*/

int Platforms::AndroidGetApiLevel()
{
#ifdef __ANDROID__
    return AndroidWrapper::GetApiLevel();
#endif
    return 0;
}

bool Platforms::AndroidShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset)
{
#ifdef __ANDROID__
    return AndroidWrapper::ShowToast(Message, Duration, Gravity, xOffset, yOffset);
#endif
    return false;
}

bool Platforms::DirExists(const char* Dir)
{
#ifdef __APPLE__
    return iOSWrapper::DirectoryExists(Dir);
#endif

#ifdef __ANDROID__
    return AndroidWrapper::DirectoryExists(Dir);
#endif

#ifdef __WIN32__
    return WindowsWrapper::DirectoryExists(Dir);
#endif

    return false;
}

bool Platforms::GetDirContent  (const char* Dir, DirContentElementArray& ArrayList)
{
#ifdef __APPLE__
    return iOSWrapper::GetDirContent(Dir, ArrayList);
#endif

#ifdef __ANDROID__
    return AndroidWrapper::GetDirContent(Dir, ArrayList);
#endif

#ifdef __WIN32__
    return WindowsWrapper::GetDirContent(Dir, ArrayList);
#endif
    return false;
}

bool Platforms::DirRemove(const char* Dir)
{
#ifdef __WIN32__
    return WindowsWrapper::DirRemove(Dir);
#endif

#ifdef __ANDROID__
    return AndroidWrapper::DirRemove(Dir);
#endif
    return false;
};

bool Platforms::DirRemoveRecursive(const char* Dir)
{
#ifdef __WIN32__
    return WindowsWrapper::DirRemoveRecursive(Dir);
#endif

#ifdef __ANDROID__
    return AndroidWrapper::DirRemoveRecursive(Dir);
#endif

#ifdef __APPLE__
    return iOSWrapper::DirRemoveRecursive(Dir);
#endif
    return false;
};

std::string Platforms::GetEnvVariable(const char* Variable)
{
#ifdef __WIN32__
    return WindowsWrapper::GetEnvVariable(Variable);
#endif
    return std::string("");
}


void Platforms::MessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, Uint32 TimeOutMS)
{
#ifdef __WIN32__
    return WindowsWrapper::MessageBoxShow(Code, Title, Message, Button1, Button2, Button3);
#endif

#ifdef __ANDROID__
    return AndroidWrapper::MessageBoxShow(Code, Title, Message, Button1, Button2, Button3, TimeOutMS);
#endif

#ifdef __APPLE__
    iOSWrapper::MessageBoxShow(Code, Title, Message, Button1, Button2, Button3);
#endif
}

void Platforms::ShareText(const char* Title, const char* Message)
{
#ifdef __ANDROID__
    return AndroidWrapper::ShareText(Title, Message);
#endif

#ifdef __APPLE__
    return iOSWrapper::ShareText(Title, Message);
#endif
};

bool Platforms::Init()
{
#ifdef __ANDROID__
    return AndroidWrapper::Init();
#endif

    return true;
};

#endif // __AKK0RD_PLATFORMS_H__
