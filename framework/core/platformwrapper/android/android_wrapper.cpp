#include "../platforms.h"
#include "customevents.h"
#include "core/core_defines.h"
#include "android_wrapper.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <dirent.h>

class AndroidPlatformWrapper : public PlatformWrapper {
    struct {
        AAssetManager *AssetMgr               { nullptr };
        jclass    UtilsClass                  { nullptr } ;
        jmethodID midDirectoryDelete          { nullptr };
        //jmethodID midOpenURL                  { nullptr };
        jmethodID midShowToast                { nullptr };
        jmethodID midMkDir                    { nullptr };
        jmethodID midShowMessageBox           { nullptr };
        jmethodID midGetAssetManager          { nullptr };
        jmethodID midShareText                { nullptr };
        //jmethodID midSharePNG                 { nullptr };
        jmethodID midGetAudioOutputRate       { nullptr };
        jmethodID midGetAudioOutputBufferSize { nullptr };
        jmethodID midLaunchAppReviewIfAvailable { nullptr };
        jmethodID midGetAppVersionInfo        { nullptr };

        std::string sLanguage;
        //std::string sInternalDir;
        std::string sInternalWriteDir;
        int         sApiLevel;
    } AndroidWrapperState;

    jclass getJavaClass(JNIEnv* Env, const char* ClassName, bool PrintTraceOnError) {
        jclass localClass = Env->FindClass(ClassName);
        if (Env->ExceptionCheck()) {
            logWarning("Java class %s not found", ClassName);
            if(PrintTraceOnError) {
                Env->ExceptionDescribe();
            }
            Env->ExceptionClear();
            return nullptr;
        }
        jclass globalClass = reinterpret_cast<jclass>(Env->NewGlobalRef(localClass));
        Env->DeleteLocalRef(localClass);
        return globalClass;
    }

    jmethodID getJavaStaticMethod(JNIEnv* Env, jclass& JavaClass, const char* MethodName, const char* Signature, bool PrintTraceOnError) {
        jmethodID method = Env->GetStaticMethodID(JavaClass, MethodName, Signature);
        if (Env->ExceptionCheck()) {
            logWarning("Java method %s not found", MethodName);
            if(PrintTraceOnError) {
                Env->ExceptionDescribe();
            }
            Env->ExceptionClear();
            return nullptr;
        }
        return method;
    }

    void InitAssetsManager() {
        if(!AndroidWrapperState.AssetMgr) {
            JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
            //jclass activity = FindAkkordClassUtils(env);
            //logDebug("GetStaticMethodID before");
            //jmethodID GetAssetManager = env->GetStaticMethodID(activity, "GetAssetManager", "()Landroid/content/res/AssetManager;");
            //logDebug("GetStaticMethodID after");
            if(!AndroidWrapperState.midGetAssetManager) {
                logError("AndroidWrapper GetAssetManager Java method not Found");
                return;
            }

            //logDebug("Call Method");
            jobject jAssetsMgr = (jobject)env->CallStaticObjectMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midGetAssetManager);
            AndroidWrapperState.AssetMgr = AAssetManager_fromJava(env, jAssetsMgr);
        }
    }

    bool private_DirRemove(const char* Dir, bool Recursive) {
        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
        //jclass activity = FindAkkordClassUtils(env);
        //logDebug("GetStaticMethodID before");
        //jmethodID DirRemove = env->GetStaticMethodID(activity, "DirectoryDelete", "(Ljava/lang/String;I)I");
        //logDebug("GetStaticMethodID after");
        if(!AndroidWrapperState.midDirectoryDelete) {
            logError("AndroidWrapper: DirectoryDelete Java method not Found");
            return false;
        }

        jstring url_jstring = (jstring)env->NewStringUTF(Dir);
        int Recurs = ((Recursive == true) ? (1) : (0));
        //logDebug("Call Method");
        jint value = env->CallStaticIntMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midDirectoryDelete, url_jstring, Recurs);
        env->DeleteLocalRef(url_jstring);
        //env->DeleteLocalRef(activity);

        int result = value;

        return ((result == 0) ? (true) : (false));
    }

    bool vInit() override {
        AndroidWrapperState.AssetMgr                    = nullptr;
        AndroidWrapperState.UtilsClass                  = nullptr;
        AndroidWrapperState.midDirectoryDelete          = nullptr;
        //AndroidWrapperState.midOpenURL                  = nullptr;
        AndroidWrapperState.midShowToast                = nullptr;
        AndroidWrapperState.midMkDir                    = nullptr;
        AndroidWrapperState.midShowMessageBox           = nullptr;
        AndroidWrapperState.midGetAssetManager          = nullptr;
        AndroidWrapperState.midShareText                = nullptr;
        //AndroidWrapperState.midSharePNG               = nullptr;
        AndroidWrapperState.midGetAudioOutputRate       = nullptr;
        AndroidWrapperState.midGetAudioOutputBufferSize = nullptr;

        bool Result = true;
        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
        AndroidWrapperState.UtilsClass = getJavaClass(env, "org/akkord/lib/Utils", true);
        if(!AndroidWrapperState.UtilsClass) {
            return false;
        }
        AndroidWrapperState.midDirectoryDelete             = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "DirectoryDelete", "(Ljava/lang/String;I)I", true);
        //AndroidWrapperState.midOpenURL                     = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "openURL", "(Ljava/lang/String;)V", true);
        AndroidWrapperState.midShowToast                   = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "showToast", "(Ljava/lang/String;IIII)V", true);
        AndroidWrapperState.midMkDir                       = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "MkDir", "(Ljava/lang/String;)I", true);
        AndroidWrapperState.midShowMessageBox              = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "showMessageBox", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)V", true);
        AndroidWrapperState.midGetAssetManager             = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "GetAssetManager", "()Landroid/content/res/AssetManager;", true);
        AndroidWrapperState.midShareText                   = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "shareText", "(Ljava/lang/String;Ljava/lang/String;)V", true);
        AndroidWrapperState.midLaunchAppReviewIfAvailable  = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "LaunchAppReviewIfAvailable", "()V", true);
        AndroidWrapperState.midGetAppVersionInfo           = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "GetAppVersionInfo", "()Ljava/lang/String;", true);
        // пока комментим, так как для Android требуется FileProvider
        //AndroidWrapperState.midSharePNG                  = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "sharePNG", "(Ljava/lang/String;Ljava/lang/String;)V", true);

        AndroidWrapperState.midGetAudioOutputRate        = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "getAudioOutputRate", "()I", true);
        AndroidWrapperState.midGetAudioOutputBufferSize  = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "getAudioOutputBufferSize", "()I", true);

        // One-time call functions
        jmethodID GetLanguage         = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "getLanguage", "()Ljava/lang/String;", true);
        jmethodID GetApiLevel         = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "GetApiLevel", "()I", true);
        jmethodID GetInternalWriteDir = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "GetInternalWriteDir", "()Ljava/lang/String;", true);
        //jmethodID GetInternalDir      = getJavaStaticMethod(env, AndroidWrapperState.UtilsClass, "GetInternalDir", "()Ljava/lang/String;", true);

        // кешируем язык
        jstring jstrLang = (jstring)env->CallStaticObjectMethod(AndroidWrapperState.UtilsClass, GetLanguage);
        const char* LangStr = env->GetStringUTFChars(jstrLang, 0);
        // global variable
        AndroidWrapperState.sLanguage = std::string(LangStr);
        env->ReleaseStringUTFChars(jstrLang, LangStr);

        // кешируем директорию для записи
        jstring jstrWriteDir = (jstring)env->CallStaticObjectMethod(AndroidWrapperState.UtilsClass, GetInternalWriteDir);
        const char* javaWriteDir = env->GetStringUTFChars(jstrWriteDir, 0);
        AndroidWrapperState.sInternalWriteDir = std::string(javaWriteDir);
        env->ReleaseStringUTFChars(jstrWriteDir, javaWriteDir);
        if(AndroidWrapperState.sInternalWriteDir.back() != '/') {
            AndroidWrapperState.sInternalWriteDir += "/";
        }

        // кешируем внутреннюю директорию
        //jstring jstrDir = (jstring)env->CallStaticObjectMethod(AndroidWrapperState.UtilsClass, GetInternalDir);
        //const char* javaDir = env->GetStringUTFChars(jstrDir, 0);
        //AndroidWrapperState.sInternalDir = std::string(javaDir);
        //env->ReleaseStringUTFChars(jstrDir, javaDir);
        //if(AndroidWrapperState.sInternalDir.back() != '/') {
        //    AndroidWrapperState.sInternalDir += "/";
        //}

        jint value = env->CallStaticIntMethod(AndroidWrapperState.UtilsClass, GetApiLevel);
        AndroidWrapperState.sApiLevel = (int)value;

        return Result;
    };

    Locale::Lang             vGetDeviceLanguage() override {
        return Locale::DecodeLang_ISO639_Code(AndroidWrapperState.sLanguage.c_str());
    };

    std::string              vGetInternalWriteDir() override {
        return AndroidWrapperState.sInternalWriteDir;
    };

    std::string              vGetInternalAssetsDir() override {
        return "";
    };

    bool vDirCreate(const char* Dir) override {
        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
        if(!AndroidWrapperState.midMkDir)
        {
            logError("AndroidWrapper MkDir Java method not Found");
            return false;
        }

        jstring url_jstring = (jstring)env->NewStringUTF(Dir);
        //logDebug("Call Method");
        jint value = env->CallStaticIntMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midMkDir, url_jstring);
        env->DeleteLocalRef(url_jstring);
        int retval = (int)value;

        switch(retval)
        {
            case 0:
                return true;
                break;
            case 1:
                logError("Directory create error %s", Dir);
                return false;
                break;
            case 2:
                logError("Directory create error %s. File with the same name exists", Dir);
                return false;
                break;
            default:
                logError("Directory create error %s. Unknown error", Dir);
                return false;
                break;
        };

        return false;
    };

    bool vDirExists(const char* Dir) override {
        DIR* dir = opendir(Dir);
        if(dir != nullptr) {
            closedir(dir);
            return true;
        }
        return false;
    };

    bool vDirRemove(const char* Dir) override {
        return private_DirRemove(Dir, false);
    };

    bool vDirRemoveRecursive(const char* Dir) override {
        return private_DirRemove(Dir, true);
    };

    bool vGetDirContent(const char* Dir, DirContentElementArray& ArrayList)  override {
        // https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
        struct dirent *entry;
        DIR *dir = opendir(Dir);

        if(dir != nullptr) {
            while ((entry = readdir(dir)) != nullptr) {
                auto Name = std::string(entry->d_name);
                if((Name != "." && Name != "..") || (DT_DIR != entry->d_type)) {
                    ArrayList.emplace_back(std::make_unique<DirContentElement>());
                    ArrayList.back()->Name  = Name;
                    ArrayList.back()->isDir = (DT_DIR == entry->d_type ? 1 : 0);
                }
            }
            closedir(dir);
        }

        return true;
    };

    // Activity functions
    /*
    bool vOpenURL(const char* url)  override {
        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
        //logDebug("GetStaticMethodID before");
        //logDebug("GetStaticMethodID after");
        if(!AndroidWrapperState.midOpenURL) {
            logError("AndroidWrapper: OpenURL Java method not Found");
            return false;
        }
        jstring url_jstring = (jstring)env->NewStringUTF(url);
        //logDebug("Call Method");
        env->CallStaticVoidMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midOpenURL, url_jstring);
        env->DeleteLocalRef(url_jstring);
        return true;
    };
    */

    void vMessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, Uint32 TimeOutMS)  override {
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

        Uint64 tm = static_cast<Uint64>(TimeOutMS);

        //logDebug("Call Method");
        env->CallStaticVoidMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midShowMessageBox, Code, jstring_Title, jstring_Message, jstring_Button1, jstring_Button2, jstring_Button3, tm);

        env->DeleteLocalRef(jstring_Title  );
        env->DeleteLocalRef(jstring_Message);
        env->DeleteLocalRef(jstring_Button1);
        env->DeleteLocalRef(jstring_Button2);
        env->DeleteLocalRef(jstring_Button3);
    };

    void vShareText(const char* Title, const char* Message) override
    {
        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
        if(!AndroidWrapperState.midShareText) {
            logError("AndroidWrapper midShareText Java method not Found");
            return;
        }

        jstring jstring_Title   = (jstring)env->NewStringUTF(Title);
        jstring jstring_Message = (jstring)env->NewStringUTF(Message);
        env->CallStaticVoidMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midShareText, jstring_Title, jstring_Message);

        env->DeleteLocalRef(jstring_Title  );
        env->DeleteLocalRef(jstring_Message);
    };

    int vGetAudioOutputRate() override {
        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
        if(!AndroidWrapperState.midGetAudioOutputRate) {
            logError("AndroidWrapper getAudioOutputRate Java method not Found");
            return -1; // return default value
        }
        jint value = env->CallStaticIntMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midGetAudioOutputRate);
        //logDebug("outputRate %d", static_cast<int>(value));
        return static_cast<int>(value);
    };

    int vGetAudioOutputBufferSize() override {
        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
        if(!AndroidWrapperState.midGetAudioOutputBufferSize) {
            logError("AndroidWrapper getAudioOutputBufferSize Java method not Found");
            return -1; // return default value
        }
        jint value = env->CallStaticIntMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midGetAudioOutputBufferSize);
        //logDebug("outputBufferSize %d", static_cast<int>(value));
        return static_cast<int>(value);
    };

    bool vLaunchAppReviewIfAvailable() override {
        if(!AndroidWrapperState.midLaunchAppReviewIfAvailable) {
            logError("AndroidWrapper LaunchAppReviewIfAvailable Java method not Found");
            return false;
        }
        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
        env->CallStaticVoidMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midLaunchAppReviewIfAvailable);
        return true;
    }

    std::string vGetAppVersionInfo() override {
        if(!AndroidWrapperState.midGetAppVersionInfo) {
            logError("AndroidWrapper GetAppVersionInfo Java method not Found");
            return "Unknown";
        }

        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
        jstring jstrVersion = (jstring)env->CallStaticObjectMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midGetAppVersionInfo);
        const char* VersionStr = env->GetStringUTFChars(jstrVersion, 0);
        const std::string versionString(VersionStr);
        env->ReleaseStringUTFChars(jstrVersion, VersionStr);
        return versionString;
    }

public:
    int GetApiLevel() {
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

    bool ShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset) {
        JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
        //logDebug("GetStaticMethodID before");
        //logDebug("GetStaticMethodID after");
        if(!AndroidWrapperState.midShowToast) {
            logError("AndroidWrapper::ShowToast Java method not Found");
            return false;
        }
        jstring url_jstring = (jstring)env->NewStringUTF(Message);
        //logDebug("Call Method");
        env->CallStaticVoidMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midShowToast, url_jstring, Duration, Gravity, xOffset, yOffset);
        env->DeleteLocalRef(url_jstring);
        return true;
    }

    char* GetAsset2Buffer(const char* FileName, unsigned& Size)
    {
        Size = 0;
        InitAssetsManager();

        if(AndroidWrapperState.AssetMgr)  {
            auto asset = AAssetManager_open(AndroidWrapperState.AssetMgr, FileName, AASSET_MODE_UNKNOWN);
            if(asset) {
                Size         = AAsset_getLength(asset);
                char* buffer = new char[Size];

                AAsset_read(asset, buffer, Size);
                AAsset_close(asset);
                return buffer;
            }
        }
        return nullptr;
    }
};

static AndroidPlatformWrapper androidPlatformWrapper;
PlatformWrapper& PlatformWrapper::vGetInstance() {
    return androidPlatformWrapper;
};

char* AndroidWrapper::GetAsset2Buffer(const char* FileName, unsigned& Size) {
    return androidPlatformWrapper.GetAsset2Buffer(FileName, Size);
}

bool AndroidWrapper::AndroidShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset) {
    return androidPlatformWrapper.ShowToast(Message, Duration, Gravity, xOffset, yOffset);
};

int AndroidWrapper::AndroidGetApiLevel() {
    return androidPlatformWrapper.GetApiLevel();
};

extern "C" {
JNIEXPORT void JNICALL Java_org_akkord_lib_Utils_MessageBoxCallback(JNIEnv*, jclass, jint, jint);
}

JNIEXPORT void JNICALL Java_org_akkord_lib_Utils_MessageBoxCallback(JNIEnv* mEnv, jclass cls, jint Code, jint Result)
{
    CustomEvents::MessageBoxCallback(Code, Result);
};