#pragma once
#ifndef __AKK0RD_ANDROID_BASEWRAPPER_H__
#define __AKK0RD_ANDROID_BASEWRAPPER_H__

#include "basewrapper.h"
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "core/core_defines.h"
#include "android_java_callbacks.h"
//#include "../core_types.h"

#include <dirent.h>

struct AndroidFilesHandleStruct
{
    jclass FileListManager;
    unsigned ItemsCount = 0;
    unsigned Pointer = 0;
};

static AAssetManager *AssetMgr = NULL;

class AndroidWrapper
{   
private:    
    static bool   private_DirRemove(const char* Dir, bool Recursive);
    static jclass FindAkkordClassUtils(JNIEnv *env);   
    
    static void              InitAssetsManager    ();
 
public:
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
	
	static void              MessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3);
};

jclass AndroidWrapper::FindAkkordClassUtils(JNIEnv *env)
{
    return env->FindClass("org/akkord/lib/Utils");    
}

bool AndroidWrapper::private_DirRemove(const char* Dir, bool Recursive)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindAkkordClassUtils(env);
	logDebug("GetStaticMethodID before");
    jmethodID DirRemove = env->GetStaticMethodID(activity, "DirectoryDelete", "(Ljava/lang/String;I)I");
	logDebug("GetStaticMethodID after");
    if(!DirRemove)
    {        
        logError("AndroidWrapper: DirectoryDelete Java method not Found");        
        return false;
    }        
    
    jstring url_jstring = (jstring)env->NewStringUTF(Dir);
    int Recurs = ((Recursive == true) ? (1) : (0));
    
    jint value = env->CallStaticIntMethod(activity, DirRemove, url_jstring, Recurs);
    env->DeleteLocalRef(url_jstring);    
    env->DeleteLocalRef(activity);     
    
    int result = value;
    
    return ((result == 0) ? (true) : (false));
}

bool AndroidWrapper::DirRemove(const char* Dir)
{
    return private_DirRemove(Dir, false);
};

bool AndroidWrapper::DirRemoveRecursive(const char* Dir)
{
    return private_DirRemove(Dir, true);
};

bool AndroidWrapper::DirectoryExists(const char* Dir)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindAkkordClassUtils(env);
	logDebug("GetStaticMethodID before");
    jmethodID DirectoryExists = env->GetStaticMethodID(activity, "DirectoryExists", "(Ljava/lang/String;)I");
	logDebug("GetStaticMethodID after");
    if(!DirectoryExists)
    {        
        logError("AndroidWrapper: DirectoryExists Java method not Found");        
        return false;
    }        
    jstring url_jstring = (jstring)env->NewStringUTF(Dir);
    jint value = env->CallStaticIntMethod(activity, DirectoryExists, url_jstring);
    env->DeleteLocalRef(url_jstring);    
    env->DeleteLocalRef(activity);   
    
    int v = value;
    if(v == 2) return true;
    else       return false;
}

bool AndroidWrapper::OpenURL(const char* url)
{    
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindAkkordClassUtils(env);
	logDebug("GetStaticMethodID before");
    jmethodID openURL = env->GetStaticMethodID(activity, "openURL", "(Ljava/lang/String;)V");
	logDebug("GetStaticMethodID after");
    if(!openURL)
    {        
        logError("AndroidWrapper: OpenURL Java method not Found");        
        return false;
    }        
    jstring url_jstring = (jstring)env->NewStringUTF(url);
    env->CallStaticVoidMethod(activity, openURL, url_jstring);
    env->DeleteLocalRef(url_jstring);    
    env->DeleteLocalRef(activity);
    return true;
}

bool AndroidWrapper::GetDirContent(const char* Dir, DirContentElementArray& ArrayList)
{    
    // https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
	struct dirent *entry;
    DIR *dir = opendir(Dir);
	
	if(dir != nullptr)
	{
		while ((entry = readdir(dir)) != nullptr)
		{
			//logDebug("%s %d\n",entry->d_name, int(entry->d_type));
			
            std::unique_ptr<DirContentElement> dc (new DirContentElement());
            dc->Name  = std::string(entry->d_name);
            dc->isDir = (DT_DIR == entry->d_type ? 1 : 0);
            ArrayList.push_back(std::move(dc));
		}
		closedir(dir);
	}
	
		
	/*
	JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();    
    jclass localClass = env->FindClass("org/akkord/lib/FileListManager"); 
    
    jmethodID JavaMethodGetList = env->GetStaticMethodID(localClass, "GetList", "(Ljava/lang/String;)I");    
    if(!JavaMethodGetList)
    {
        logError("AndroidWrapper::GetDirContent: GetList Java method not Found");        
        return false;        
    }    
    
    jstring Path = (jstring)env->NewStringUTF(Dir);  
    jint value = env->CallStaticIntMethod(localClass, JavaMethodGetList, Path);
    env->DeleteLocalRef(Path);
    unsigned Count = (unsigned)value;
    
    jmethodID JavaMethodFName = env->GetStaticMethodID(localClass, "GetFileName", "(I)Ljava/lang/String;");
    if(!JavaMethodFName)
    {
        logError("AndroidWrapper::GetDirContent: GetFileName Java method not Found");        
        return false;        
    }        
    
    jmethodID JavaMethodIsDir = env->GetStaticMethodID(localClass, "GetIsDir", "(I)I");
    if(!JavaMethodIsDir)
    {
        logError("AndroidWrapper::GetDirContent: GetIsDir Java method not Found");        
        return false;        
    }

    jmethodID JavaMethodCloseList = env->GetStaticMethodID(localClass, "CloseList", "()V");
    if(!JavaMethodCloseList)
    {
        logError("AndroidWrapper::CloseList Java method not Found");        
        return false;        
    }         
    
    for(unsigned i = 0; i < Count; i++)
    {
        jstring jstr         = (jstring)env->CallStaticObjectMethod(localClass, JavaMethodFName, i);
        const char* FileName = env->GetStringUTFChars(jstr, 0);
        
        jint IsDir           = env->CallStaticIntMethod(localClass, JavaMethodIsDir, i);        
        
        std::unique_ptr<DirContentElement> dc (new DirContentElement());
        dc->Name  = std::string(FileName);
        dc->isDir = IsDir;        
        ArrayList.push_back(std::move(dc));
        
        env->ReleaseStringUTFChars(jstr, FileName);
    }

    env->CallStaticVoidMethod(localClass, JavaMethodCloseList);        
    env->DeleteLocalRef(localClass);    
    */
    return true;
}

bool AndroidWrapper::ShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset)
{    
    // https://developer.android.com/reference/android/view/Gravity.html
    // https://stackoverflow.com/questions/8001863/android-set-toast-to-the-default-position-centered-just-above-the-status-bar
    // http://developer.alexanderklimov.ru/android/toast.php
        
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindAkkordClassUtils(env);
	logDebug("GetStaticMethodID before");
    jmethodID showToastJava = env->GetStaticMethodID(activity, "showToast", "(Ljava/lang/String;IIII)V");    
	logDebug("GetStaticMethodID after");
    if(!showToastJava)
    {        
        logError("AndroidWrapper::ShowToast Java method not Found");        
        return false;
    }    
    jstring url_jstring = (jstring)env->NewStringUTF(Message);
    env->CallStaticVoidMethod(activity, showToastJava, url_jstring, Duration, Gravity, xOffset, yOffset);    
    env->DeleteLocalRef(url_jstring);        
    env->DeleteLocalRef(activity);      
    return true;
}

int AndroidWrapper::GetApiLevel()
{
    // https://stackoverflow.com/questions/10196361/how-to-check-the-device-running-api-level-using-c-code-via-ndk
    // https://stackoverflow.com/questions/19355783/getting-os-version-with-ndk-in-c
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();    
    jclass activity = FindAkkordClassUtils(env);   
	logDebug("GetStaticMethodID before");	
    jmethodID GetApiLevelJava = env->GetStaticMethodID(activity, "GetApiLevel", "()I"); 
	logDebug("GetStaticMethodID after");
    if(!GetApiLevelJava)
    {        
        logError("AndroidWrapper GetApiLevel Java method not Found");        
        return 0;
    }
    
    jint value = env->CallStaticIntMethod(activity, GetApiLevelJava);        
    int retval = (int)value;
    env->DeleteLocalRef(activity);        
    return retval;
};

bool AndroidWrapper::DirCreate(const char* Path)
{    
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv(); 
    jclass activity = FindAkkordClassUtils(env);
	logDebug("GetStaticMethodID before");
    jmethodID MkDir = env->GetStaticMethodID(activity, "MkDir", "(Ljava/lang/String;)I");    
	logDebug("GetStaticMethodID after");
    if(!MkDir)
    {        
        logError("AndroidWrapper MkDir Java method not Found");        
        return false;
    }        
    
    jstring url_jstring = (jstring)env->NewStringUTF(Path);    
    jint value = env->CallStaticIntMethod(activity, MkDir, url_jstring);    
    env->DeleteLocalRef(url_jstring);
    int retval = (int)value;    
    env->DeleteLocalRef(activity);                
    
    switch(retval)
    {
        case 0:
            return true;
            break;
        case 1:
            logError("Directory create error %s", Path); 
            return false;
            break;
        case 2:
            logError("Directory create error %s. File with the same name exists", Path); 
            return false;
            break;  
        default:
            logError("Directory create error %s. Unknown error", Path); 
            return false;
            break;
    };
    
    return false;
}

std::string AndroidWrapper::GetLanguage()
{       
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();    
    jclass activity = FindAkkordClassUtils(env);    
	logDebug("GetStaticMethodID before");
    jmethodID getLanguage = env->GetStaticMethodID(activity, "getLanguage", "()Ljava/lang/String;");            
	logDebug("GetStaticMethodID after");
    if(!getLanguage)
    {        
        logError("AndroidWrapper getLanguage Java method not Found");        
        return "+-";
    } 
    
	logDebug("CallStaticObjectMethod before");
    jstring jstr = (jstring)env->CallStaticObjectMethod(activity, getLanguage);
	logDebug("CallStaticObjectMethod after");
    const char* LangStr = env->GetStringUTFChars(jstr, 0);
    
    std::string Lang(LangStr);
    
    env->ReleaseStringUTFChars(jstr, LangStr);
	env->DeleteLocalRef(activity);
    logDebug("GetLanguage()");
    return Lang;
};

void AndroidWrapper::InitAssetsManager()
{    
    if(!AssetMgr) 
    {   
        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();        
        jclass activity = FindAkkordClassUtils(env);    
		logDebug("GetStaticMethodID before");		
        jmethodID GetAssetManager = env->GetStaticMethodID(activity, "GetAssetManager", "()Landroid/content/res/AssetManager;");
		logDebug("GetStaticMethodID after");
        if(!GetAssetManager)
        {        
            logError("AndroidWrapper GetAssetManager Java method not Found");        
            return;
        }
        logDebug("CallStaticObjectMethod before");
        jobject jAssetsMgr = (jobject)env->CallStaticObjectMethod(activity, GetAssetManager);
		logDebug("CallStaticObjectMethod after");
        AssetMgr = AAssetManager_fromJava(env, jAssetsMgr);
		env->DeleteLocalRef(activity);
    }
};

char* AndroidWrapper::GetAsset2Buffer(const char* FileName, unsigned& Size)
{
    Size = 0;
    InitAssetsManager();
    
    if(AssetMgr)
    {
        auto asset = AAssetManager_open(AssetMgr, FileName, AASSET_MODE_UNKNOWN);    
        if(asset)
        {
            Size         = AAsset_getLength(asset);
            char* buffer = new char[Size];

            AAsset_read(asset, buffer, Size);
            AAsset_close(asset);
            return buffer;    
        }
    }
    return nullptr;
}

void AndroidWrapper::GetInternalDirs(std::string& InternalDir, std::string& InternalWriteDir)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();        
    jclass activity = FindAkkordClassUtils(env);
	logDebug("GetStaticMethodID before");
    jmethodID GetDir = env->GetStaticMethodID(activity, "GetInternalWriteDir", "()Ljava/lang/String;");        
	logDebug("GetStaticMethodID after");
    if(!GetDir)
    {        
        logError("AndroidWrapper GetDir Java method not Found");        
        return;
    }     
    
    logDebug("CallStaticObjectMethod before");
	jstring jstr = (jstring)env->CallStaticObjectMethod(activity, GetDir);   
	logDebug("CallStaticObjectMethod after");	
    const char* javaDir = env->GetStringUTFChars(jstr, 0);      
    InternalWriteDir = std::string(javaDir);    
    env->ReleaseStringUTFChars(jstr, javaDir);
    
    logDebug("GetStaticMethodID before");
	GetDir = env->GetStaticMethodID(activity, "GetInternalDir", "()Ljava/lang/String;");    
	logDebug("GetStaticMethodID after");
    if(!GetDir)
    {        
        logError("AndroidWrapper GetInternalDir Java method not Found");        
        return;
    }         
    logDebug("CallStaticObjectMethod before");
    jstr = (jstring)env->CallStaticObjectMethod(activity, GetDir);
	logDebug("CallStaticObjectMethod after");	
    const char* javaInternalDir = env->GetStringUTFChars(jstr, 0);
    InternalDir = std::string(javaInternalDir);
    env->ReleaseStringUTFChars(jstr, javaInternalDir);    
    
    env->DeleteLocalRef(activity);       
    
    if(InternalWriteDir[InternalWriteDir.length() - 1] != '/')
        InternalWriteDir = InternalWriteDir + "/";

    if(InternalDir[InternalDir.length() - 1] != '/')
        InternalDir = InternalDir + "/";    
}

void AndroidWrapper::MessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();        
    jclass activity = FindAkkordClassUtils(env);
    logDebug("GetStaticMethodID before");
	jmethodID ShowMessageBox = env->GetStaticMethodID(activity, "showMessageBox", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	logDebug("GetStaticMethodID after");
	if(!ShowMessageBox)
	{
		logError("AndroidWrapper showMessageBox Java method not Found");
		return;
	}
	
	jstring jstring_Title   = (jstring)env->NewStringUTF(Title);
	jstring jstring_Message = (jstring)env->NewStringUTF(Message);
	jstring jstring_Button1 = (jstring)env->NewStringUTF(Button1);
	jstring jstring_Button2 = (jstring)env->NewStringUTF(Button2);
	jstring jstring_Button3 = (jstring)env->NewStringUTF(Button3);
	
	env->CallStaticVoidMethod(activity, ShowMessageBox, Code, jstring_Title, jstring_Message, jstring_Button1, jstring_Button2, jstring_Button3);
	
	env->DeleteLocalRef(jstring_Title  );
	env->DeleteLocalRef(jstring_Message);
	env->DeleteLocalRef(jstring_Button1);
	env->DeleteLocalRef(jstring_Button2);
	env->DeleteLocalRef(jstring_Button3);
	
	env->DeleteLocalRef(activity);
};

#endif // __AKK0RD_ANDROID_BASEWRAPPER_H__