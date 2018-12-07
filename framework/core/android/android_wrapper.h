#ifndef __AKK0RD_ANDROID_BASEWRAPPER_H__
#define __AKK0RD_ANDROID_BASEWRAPPER_H__

#include "basewrapper.h"
#include <jni.h>

class AndroidWrapper
{   
private:    
    static bool   private_DirRemove(const char* Dir, bool Recursive);
    //static jclass FindAkkordClassUtils(JNIEnv *env);   
    
    static void              InitAssetsManager    ();
 
public:
    typedef void (onActivityResultCallback)(JNIEnv *env, jobject thiz, jobject activity, jint request_code, jint result_code, jobject data);
    
    static bool              Init();
	static bool              OpenURL              (const char* url);    
    static std::string       GetLanguage          ();
    static int               GetApiLevel          ();    
    
    static bool              DirCreate            (const char* Path);                                 
    static bool              DirectoryExists      (const char* Dir);
    static bool              DirRemove            (const char* Dir);
    static bool              DirRemoveRecursive   (const char* Dir);    
    //static bool              FileExists           (const char* FileName);
    
    static char*             GetAsset2Buffer      (const char* FileName, unsigned& Size);
    
    static void              GetInternalDirs      (std::string& InternalDir, std::string& InternalWriteDir);
    
    static bool              ShowToast            (const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset);
    static bool              GetDirContent        (const char* Dir, DirContentElementArray& ArrayList);
	
	static void              MessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, Uint32 TimeOutMS);
};

#endif // __AKK0RD_ANDROID_BASEWRAPPER_H__