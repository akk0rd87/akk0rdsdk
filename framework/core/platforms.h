#pragma once
#ifndef __AKK0RD_SDK_PLATFORMS_H__
#define __AKK0RD_SDK_PLATFORMS_H__

#include "locale.h"
#include "../basewrapper.h"

class PlatformWrapper
{
public:
    static                   PlatformWrapper& GetInstance() { return vGetInstance(); }

    bool                     Init() { return vInit(); }
    Locale::Lang             GetDeviceLanguage() { return vGetDeviceLanguage(); }
    std::string              GetEnvVariable(const char* Variable) { return vGetEnvVariable(Variable); } // Only for windows

    //std::string              GetInternalDir() { return vGetInternalDir(); };
    std::string              GetInternalWriteDir() { return vGetInternalWriteDir(); }
    std::string              GetInternalAssetsDir() { return vGetInternalAssetsDir(); }

    bool                     DirCreate(const char* Dir) { return vDirCreate(Dir); };
    bool                     DirExists(const char* Dir) { return vDirExists(Dir); };
    bool                     DirRemove(const char* Dir) { return vDirRemove(Dir); };
    bool                     DirRemoveRecursive(const char* Dir) { return vDirRemoveRecursive(Dir); };
    bool                     GetDirContent(const char* Dir, DirContentElementArray& ArrayList) { return vGetDirContent(Dir, ArrayList); };

    // Activity functions
    bool                     OpenURL(const char* url) { return vOpenURL(url); };
    void                     MessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, Uint32 TimeOutMS) { return vMessageBoxShow(Code, Title, Message, Button1, Button2, Button3, TimeOutMS); };
    void                     ShareText(const char* Title, const char* Message) { vShareText(Title, Message); }; // mobile platforms only
    void                     SharePNG(const char* Title, const char* File) { vSharePNG(Title, File); }; // mobile platforms only

    int                      GetAudioOutputRate() { return vGetAudioOutputRate(); };       // only for Android
    int                      GetAudioOutputBufferSize() { return vGetAudioOutputBufferSize(); }; // only for Android
    bool                     LaunchAppReviewIfAvailable() { return vLaunchAppReviewIfAvailable(); }

private:
    static                           PlatformWrapper& vGetInstance();

    virtual bool                     vInit() = 0;
    virtual Locale::Lang             vGetDeviceLanguage() = 0;
    virtual std::string              vGetEnvVariable(const char* Variable) { return ""; }; // Only for windows

    //virtual std::string              vGetInternalDir() = 0;
    virtual std::string              vGetInternalWriteDir() = 0;
    virtual std::string              vGetInternalAssetsDir() { return "assets/"; };

    virtual bool                     vDirCreate(const char* Dir) = 0;
    virtual bool                     vDirExists(const char* Dir) = 0;
    virtual bool                     vDirRemove(const char* Dir) = 0;
    virtual bool                     vDirRemoveRecursive(const char* Dir) = 0;
    virtual bool                     vGetDirContent(const char* Dir, DirContentElementArray& ArrayList) = 0;

    // Activity functions
    virtual bool                     vOpenURL(const char* url) = 0;
    virtual void                     vMessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, Uint32 TimeOutMS) = 0;
    virtual void                     vShareText(const char* Title, const char* Message) {};
    virtual void                     vSharePNG(const char* Title, const char* File) {};

    // Android-specific functions
    virtual int                      vGetAudioOutputRate() { return -1; };       // only for Android
    virtual int                      vGetAudioOutputBufferSize() { return -1; }; // only for Android
    virtual bool                     vLaunchAppReviewIfAvailable() { return false; }
};

#ifdef __ANDROID__
#include "android/android_wrapper.h"
#endif

class PlatformSpecific {
public:
    static bool AndroidShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset) {
#ifdef __ANDROID__
        return AndroidWrapper::AndroidShowToast(Message, Duration, Gravity, xOffset, yOffset);
#else
        return false;
#endif
    };

    static int AndroidGetApiLevel() {
#ifdef __ANDROID__
        return AndroidWrapper::AndroidGetApiLevel();
#else
        return 0;
#endif
    };
};

#endif // __AKK0RD_SDK_PLATFORMS_H__