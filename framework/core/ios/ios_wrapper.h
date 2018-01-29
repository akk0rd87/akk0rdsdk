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
};

#endif // __AKK0RD_IOS_BASEWRAPPER_H__
