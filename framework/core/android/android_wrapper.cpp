#include "customevents.h"
#include "core/android/android_wrapper.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "core/core_defines.h"
//#include "../core_types.h"

#include <dirent.h>

struct AndroidWrapperStateStruct {
jclass    globalUtils             = nullptr;
AAssetManager *AssetMgr           = nullptr;

jmethodID midDirectoryDelete      = nullptr;
jmethodID midOpenURL              = nullptr;
jmethodID midShowToast            = nullptr;
jmethodID midMkDir                = nullptr;
jmethodID midShowMessageBox       = nullptr;
jmethodID midGetAssetManager      = nullptr;

std::string sLanguage;
std::string sInternalDir;
std::string sInternalWriteDir;
int         sApiLevel;

AndroidWrapper::onActivityResultCallback* ActivityResultCallback = nullptr;
};
static AndroidWrapperStateStruct AndroidWrapperState;

bool AndroidWrapper::Init()
{
    bool Result = true;
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass localClass = env->FindClass("org/akkord/lib/Utils");
    AndroidWrapperState.globalUtils = reinterpret_cast<jclass>(env->NewGlobalRef(localClass));
    env->DeleteLocalRef(localClass);

    AndroidWrapperState.midDirectoryDelete = env->GetStaticMethodID(AndroidWrapperState.globalUtils, "DirectoryDelete", "(Ljava/lang/String;I)I");
    AndroidWrapperState.midOpenURL         = env->GetStaticMethodID(AndroidWrapperState.globalUtils, "openURL", "(Ljava/lang/String;)V");
    AndroidWrapperState.midShowToast       = env->GetStaticMethodID(AndroidWrapperState.globalUtils, "showToast", "(Ljava/lang/String;IIII)V");
    AndroidWrapperState.midMkDir           = env->GetStaticMethodID(AndroidWrapperState.globalUtils, "MkDir", "(Ljava/lang/String;)I");
    AndroidWrapperState.midShowMessageBox  = env->GetStaticMethodID(AndroidWrapperState.globalUtils, "showMessageBox", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    AndroidWrapperState.midGetAssetManager = env->GetStaticMethodID(AndroidWrapperState.globalUtils, "GetAssetManager", "()Landroid/content/res/AssetManager;");

    if(AndroidWrapperState.midDirectoryDelete  == nullptr) { Result = false; logError("midDirectoryDelete Java method not found");}
    if(AndroidWrapperState.midOpenURL          == nullptr) { Result = false; logError("midOpenURL         Java method not found");}
    if(AndroidWrapperState.midShowToast        == nullptr) { Result = false; logError("midShowToast       Java method not found");}
    if(AndroidWrapperState.midMkDir            == nullptr) { Result = false; logError("midMkDir           Java method not found");}
    if(AndroidWrapperState.midShowMessageBox   == nullptr) { Result = false; logError("midShowMessageBox  Java method not found");}
    if(AndroidWrapperState.midGetAssetManager  == nullptr) { Result = false; logError("midGetAssetManager Java method not found");}

    // One-time call functions
    jmethodID GetLanguage         = env->GetStaticMethodID(AndroidWrapperState.globalUtils, "getLanguage", "()Ljava/lang/String;");
    jmethodID GetApiLevel         = env->GetStaticMethodID(AndroidWrapperState.globalUtils, "GetApiLevel", "()I");
    jmethodID GetInternalWriteDir = env->GetStaticMethodID(AndroidWrapperState.globalUtils, "GetInternalWriteDir", "()Ljava/lang/String;");
    jmethodID GetInternalDir      = env->GetStaticMethodID(AndroidWrapperState.globalUtils, "GetInternalDir", "()Ljava/lang/String;");    

    if(GetLanguage         == nullptr) { Result = false; logError("GetLanguage         Java method not found");   }
    if(GetApiLevel         == nullptr) { Result = false; logError("GetApiLevel         Java method not found");   }
    if(GetInternalWriteDir == nullptr) { Result = false; logError("GetInternalWriteDir Java method not found"); }
    if(GetInternalDir      == nullptr) { Result = false; logError("GetInternalDir      Java method not found"); }
    
    // кешируем язык
    jstring jstrLang = (jstring)env->CallStaticObjectMethod(AndroidWrapperState.globalUtils, GetLanguage);
    const char* LangStr = env->GetStringUTFChars(jstrLang, 0);
    // global variable
    AndroidWrapperState.sLanguage = std::string(LangStr);
    env->ReleaseStringUTFChars(jstrLang, LangStr);
    
    // кешируем директорию для записи
    jstring jstrWriteDir = (jstring)env->CallStaticObjectMethod(AndroidWrapperState.globalUtils, GetInternalWriteDir);
    const char* javaWriteDir = env->GetStringUTFChars(jstrWriteDir, 0);
    AndroidWrapperState.sInternalWriteDir = std::string(javaWriteDir);
    env->ReleaseStringUTFChars(jstrWriteDir, javaWriteDir);
    if(AndroidWrapperState.sInternalWriteDir[AndroidWrapperState.sInternalWriteDir.length() - 1] != '/')
        AndroidWrapperState.sInternalWriteDir = AndroidWrapperState.sInternalWriteDir + "/";
    
    // кешируем внутреннюю директорию
    jstring jstrDir = (jstring)env->CallStaticObjectMethod(AndroidWrapperState.globalUtils, GetInternalDir);
    const char* javaDir = env->GetStringUTFChars(jstrDir, 0);
    AndroidWrapperState.sInternalDir = std::string(javaDir);
    env->ReleaseStringUTFChars(jstrDir, javaDir);
    if(AndroidWrapperState.sInternalDir[AndroidWrapperState.sInternalDir.length() - 1] != '/')
        AndroidWrapperState.sInternalDir = AndroidWrapperState.sInternalDir + "/";
    
    jint value = env->CallStaticIntMethod(AndroidWrapperState.globalUtils, GetApiLevel);
    AndroidWrapperState.sApiLevel = (int)value;
    
    return Result;
}

//jclass AndroidWrapper::FindAkkordClassUtils(JNIEnv *env)
//{
//    return env->FindClass("org/akkord/lib/Utils");    
//}

bool AndroidWrapper::private_DirRemove(const char* Dir, bool Recursive)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    //jclass activity = FindAkkordClassUtils(env);
    logDebug("GetStaticMethodID before");
    //jmethodID DirRemove = env->GetStaticMethodID(activity, "DirectoryDelete", "(Ljava/lang/String;I)I");
    logDebug("GetStaticMethodID after");
    if(!AndroidWrapperState.midDirectoryDelete)
    {        
        logError("AndroidWrapper: DirectoryDelete Java method not Found");        
        return false;
    }        
    
    jstring url_jstring = (jstring)env->NewStringUTF(Dir);
    int Recurs = ((Recursive == true) ? (1) : (0));
    logDebug("Call Method");
    jint value = env->CallStaticIntMethod(AndroidWrapperState.globalUtils, AndroidWrapperState.midDirectoryDelete, url_jstring, Recurs);
    env->DeleteLocalRef(url_jstring);    
    //env->DeleteLocalRef(activity);     
    
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
    /*
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();        
    if(!midDirectoryExists)
    {        
        logError("AndroidWrapper: DirectoryExists Java method not Found");        
        return false;
    }        
    jstring url_jstring = (jstring)env->NewStringUTF(Dir);
    logDebug("Call Method");
    jint value = env->CallStaticIntMethod(globalUtils, midDirectoryExists, url_jstring);
    env->DeleteLocalRef(url_jstring);
    
    int v = value;
    if(v == 2) return true;
    else       return false;
    */
    DIR* dir = opendir(Dir);
    if(dir != nullptr)
    {
        closedir(dir);
        return true;
    }
    return false;
}

bool AndroidWrapper::OpenURL(const char* url)
{    
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();    
    logDebug("GetStaticMethodID before");    
    logDebug("GetStaticMethodID after");
    if(!AndroidWrapperState.midOpenURL)
    {        
        logError("AndroidWrapper: OpenURL Java method not Found");        
        return false;
    }        
    jstring url_jstring = (jstring)env->NewStringUTF(url);
    logDebug("Call Method");
    env->CallStaticVoidMethod(AndroidWrapperState.globalUtils, AndroidWrapperState.midOpenURL, url_jstring);
    env->DeleteLocalRef(url_jstring);
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
            auto Name = std::string(entry->d_name);
            if((Name != "." && Name != "..") || (DT_DIR != entry->d_type))
            {
                std::unique_ptr<DirContentElement> dc (new DirContentElement());
                dc->Name  = Name;
                dc->isDir = (DT_DIR == entry->d_type ? 1 : 0);
                ArrayList.push_back(std::move(dc));
            }
        }
        closedir(dir);
    }
    
    return true;
}

bool AndroidWrapper::ShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset)
{    
    // https://developer.android.com/reference/android/view/Gravity.html
    // https://stackoverflow.com/questions/8001863/android-set-toast-to-the-default-position-centered-just-above-the-status-bar
    // http://developer.alexanderklimov.ru/android/toast.php
        
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();    
    logDebug("GetStaticMethodID before");    
    logDebug("GetStaticMethodID after");
    if(!AndroidWrapperState.midShowToast)
    {        
        logError("AndroidWrapper::ShowToast Java method not Found");        
        return false;
    }    
    jstring url_jstring = (jstring)env->NewStringUTF(Message);
    logDebug("Call Method");
    env->CallStaticVoidMethod(AndroidWrapperState.globalUtils, AndroidWrapperState.midShowToast, url_jstring, Duration, Gravity, xOffset, yOffset);
    env->DeleteLocalRef(url_jstring);
    return true;
}

int AndroidWrapper::GetApiLevel()
{
    // https://stackoverflow.com/questions/10196361/how-to-check-the-device-running-api-level-using-c-code-via-ndk
    // https://stackoverflow.com/questions/19355783/getting-os-version-with-ndk-in-c
    /*
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();    
    if(!midGetApiLevel)
    {        
        logError("AndroidWrapper GetApiLevel Java method not Found");        
        return 0;
    }
    logDebug("Call Method");
    jint value = env->CallStaticIntMethod(globalUtils, midGetApiLevel);        
    int retval = (int)value;    
    return retval;
    */
    return AndroidWrapperState.sApiLevel;
};

bool AndroidWrapper::DirCreate(const char* Path)
{    
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();         
    if(!AndroidWrapperState.midMkDir)
    {        
        logError("AndroidWrapper MkDir Java method not Found");        
        return false;
    }        
    
    jstring url_jstring = (jstring)env->NewStringUTF(Path);    
    logDebug("Call Method");
    jint value = env->CallStaticIntMethod(AndroidWrapperState.globalUtils, AndroidWrapperState.midMkDir, url_jstring);
    env->DeleteLocalRef(url_jstring);
    int retval = (int)value;         
    
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
    /*
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    if(!midGetLanguage)
    {        
        logError("AndroidWrapper getLanguage Java method not Found");        
        return "+-";
    } 
    
    jstring jstr = (jstring)env->CallStaticObjectMethod(globalUtils, midGetLanguage);    
    const char* LangStr = env->GetStringUTFChars(jstr, 0);
    
    std::string Lang(LangStr);
    env->ReleaseStringUTFChars(jstr, LangStr);        
    return Lang;
    */
    return AndroidWrapperState.sLanguage;
};

void AndroidWrapper::InitAssetsManager()
{    
    if(!AndroidWrapperState.AssetMgr)
    {   
        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();        
        //jclass activity = FindAkkordClassUtils(env);    
        logDebug("GetStaticMethodID before");        
        //jmethodID GetAssetManager = env->GetStaticMethodID(activity, "GetAssetManager", "()Landroid/content/res/AssetManager;");
        logDebug("GetStaticMethodID after");
        if(!AndroidWrapperState.midGetAssetManager)
        {        
            logError("AndroidWrapper GetAssetManager Java method not Found");        
            return;
        }
        
        logDebug("Call Method");
        jobject jAssetsMgr = (jobject)env->CallStaticObjectMethod(AndroidWrapperState.globalUtils, AndroidWrapperState.midGetAssetManager);
        AndroidWrapperState.AssetMgr = AAssetManager_fromJava(env, jAssetsMgr);
    }
};

char* AndroidWrapper::GetAsset2Buffer(const char* FileName, unsigned& Size)
{
    Size = 0;
    InitAssetsManager();
    
    if(AndroidWrapperState.AssetMgr)
    {
        auto asset = AAssetManager_open(AndroidWrapperState.AssetMgr, FileName, AASSET_MODE_UNKNOWN);
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
    /*
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    if(!midGetInternalWriteDir)
    {        
        logError("AndroidWrapper GetDir Java method not Found");        
        return;
    }
    logDebug("Call Method");
    jstring jstr = (jstring)env->CallStaticObjectMethod(globalUtils, midGetInternalWriteDir);    
    const char* javaDir = env->GetStringUTFChars(jstr, 0);      
    InternalWriteDir = std::string(javaDir);    
    env->ReleaseStringUTFChars(jstr, javaDir);
    
    if(!midGetInternalDir)
    {        
        logError("AndroidWrapper GetInternalDir Java method not Found");        
        return;
    }         
    
    logDebug("Call Method");
    jstr = (jstring)env->CallStaticObjectMethod(globalUtils, midGetInternalDir);    
    const char* javaInternalDir = env->GetStringUTFChars(jstr, 0);
    InternalDir = std::string(javaInternalDir);
    env->ReleaseStringUTFChars(jstr, javaInternalDir);    
    
    if(InternalWriteDir[InternalWriteDir.length() - 1] != '/')
        InternalWriteDir = InternalWriteDir + "/";

    if(InternalDir[InternalDir.length() - 1] != '/')
        InternalDir = InternalDir + "/";    
    */
    
    InternalDir      = AndroidWrapperState.sInternalDir;
    InternalWriteDir = AndroidWrapperState.sInternalWriteDir;
}

void AndroidWrapper::MessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    if(!AndroidWrapperState.midShowMessageBox)
    {
        logError("AndroidWrapper showMessageBox Java method not Found");
        return;
    }
    
    jstring jstring_Title   = (jstring)env->NewStringUTF(Title);
    jstring jstring_Message = (jstring)env->NewStringUTF(Message);
    jstring jstring_Button1 = (jstring)env->NewStringUTF(Button1);
    jstring jstring_Button2 = (jstring)env->NewStringUTF(Button2);
    jstring jstring_Button3 = (jstring)env->NewStringUTF(Button3);
    
    logDebug("Call Method");
    env->CallStaticVoidMethod(AndroidWrapperState.globalUtils, AndroidWrapperState.midShowMessageBox, Code, jstring_Title, jstring_Message, jstring_Button1, jstring_Button2, jstring_Button3);
    
    env->DeleteLocalRef(jstring_Title  );
    env->DeleteLocalRef(jstring_Message);
    env->DeleteLocalRef(jstring_Button1);
    env->DeleteLocalRef(jstring_Button2);
    env->DeleteLocalRef(jstring_Button3);    
};

void AndroidWrapper::SetOnActivityResultCallback(AndroidWrapper::onActivityResultCallback* Callback)
{
    AndroidWrapperState.ActivityResultCallback = Callback;
};

extern "C" {
JNIEXPORT void JNICALL Java_org_akkord_lib_AkkordActivity_nativeOnActivityResult(JNIEnv *, jobject, jobject, jint, jint, jobject);
JNIEXPORT void JNICALL Java_org_akkord_lib_Utils_MessageBoxCallback(JNIEnv*, jclass, jint, jint);
}

JNIEXPORT void JNICALL Java_org_akkord_lib_AkkordActivity_nativeOnActivityResult(JNIEnv *env, jobject thiz, jobject activity, jint request_code, jint result_code, jobject data)
{
    logDebug("Java_org_akkord_lib_AkkordActivity_nativeOnActivityResult");
    if(AndroidWrapperState.ActivityResultCallback != nullptr)
    {
        logDebug("Callback...");
        AndroidWrapperState.ActivityResultCallback(env, thiz, activity, request_code, result_code, data);
    }
}

JNIEXPORT void JNICALL Java_org_akkord_lib_Utils_MessageBoxCallback(JNIEnv* mEnv, jclass cls, jint Code, jint Result)
{
    CustomEvents::MessageBoxCallback(Code, Result);
};