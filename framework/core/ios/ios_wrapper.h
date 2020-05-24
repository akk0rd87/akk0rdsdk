#pragma once
#ifndef __AKK0RD_IOS_BASEWRAPPER_H__
#define __AKK0RD_IOS_BASEWRAPPER_H__
#include "basewrapper.h"
#include "../core_defines.h"

class iOSWrapper
{
    public:
    static bool             OpenURL            (const char* url);
    static void             GetInternalWriteDir(std::string& Dir);
    static bool             DirectoryExists    (const char* Dir);
    static bool             FileExists         (const char* FileName);
    static bool             DirCreate          (const char* Dir);

    static bool             DirRemoveRecursive (const char* Dir);

    static bool             GetDirContent      (const char* Dir, DirContentElementArray& ArrayList);
    static std::string      GetLanguage        ();

    static void             MessageBoxShow     (int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3);
    static void             ShareText(const char* Title, const char* Message);
    static void             SharePNG(const char* Title, const char* File);
};

#endif // __AKK0RD_IOS_BASEWRAPPER_H__
