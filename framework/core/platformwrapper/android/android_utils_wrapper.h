#ifndef __AKK0RD_SDK_ANDROID_UTILS_WRAPPER_H__
#define __AKK0RD_SDK_ANDROID_UTILS_WRAPPER_H__

#include <memory>
#include <string>
#include <cstdint>
#include <istream>
#include <streambuf>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "android_javautf8_string.h"

#ifndef logError
#define logError(fmt, ...)
#endif

#ifndef logWarning
#define logWarning(fmt, ...)
#endif

class MessageBoxCallback {
public:
    virtual void onResult(int Code, int Result) = 0;
    virtual ~MessageBoxCallback() = default;
};

class AndroidUtilsWrapper {
public:
    AndroidUtilsWrapper(JNIEnv* jniEnvironment, MessageBoxCallback* msgBoxCallback = nullptr) : jniEnv(jniEnvironment) {
        callback = msgBoxCallback;
        AndroidWrapperState.UtilsClass = getJavaClass(jniEnv, "org/akkord/lib/Utils", true);
        if (!AndroidWrapperState.UtilsClass) {
            return;
        }
        AndroidWrapperState.midDirectoryDelete = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "DirectoryDelete", "(Ljava/lang/String;I)I", true);
        AndroidWrapperState.midShowToast = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "showToast", "(Ljava/lang/String;IIII)V", true);
        AndroidWrapperState.midMkDir = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "MkDir", "(Ljava/lang/String;)I", true);
        AndroidWrapperState.midShowMessageBox = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "showMessageBox", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)V", true);
        AndroidWrapperState.midGetAssetManager = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "GetAssetManager", "()Landroid/content/res/AssetManager;", true);
        AndroidWrapperState.midShareText = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "shareText", "(Ljava/lang/String;Ljava/lang/String;)V", true);
        AndroidWrapperState.midLaunchAppReviewIfAvailable = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "LaunchAppReviewIfAvailable", "()V", true);
        AndroidWrapperState.midRequestFlexibleUpdateIfAvailable = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "RequestFlexibleUpdateIfAvailable", "()V", true);
        AndroidWrapperState.midOpenURL = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "openURL", "(Ljava/lang/String;)V", true);

        AndroidWrapperState.midGetAppVersionCode = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "GetAppVersionCode", "()Ljava/lang/String;", true);
        AndroidWrapperState.midGetAppVersionName = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "GetAppVersionName", "()Ljava/lang/String;", true);
        // пока комментим, так как для Android требуется FileProvider
        //AndroidWrapperState.midSharePNG                  = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "sharePNG", "(Ljava/lang/String;Ljava/lang/String;)V", true);

        AndroidWrapperState.midGetAudioOutputRate = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "getAudioOutputRate", "()I", true);
        AndroidWrapperState.midGetAudioOutputBufferSize = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "getAudioOutputBufferSize", "()I", true);

        // One-time call functions
        jmethodID GetLanguage = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "getLanguage", "()Ljava/lang/String;", true);
        jmethodID GetApiLevel = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "GetApiLevel", "()I", true);
        jmethodID GetInternalWriteDir = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "GetInternalWriteDir", "()Ljava/lang/String;", true);
        //jmethodID GetInternalDir      = getJavaStaticMethod(jniEnv, AndroidWrapperState.UtilsClass, "GetInternalDir", "()Ljava/lang/String;", true);

        // кешируем язык
        jstring jstrLang = (jstring)jniEnv->CallStaticObjectMethod(AndroidWrapperState.UtilsClass, GetLanguage);
        const char* LangStr = jniEnv->GetStringUTFChars(jstrLang, 0);
        // global variable
        AndroidWrapperState.sLanguage = std::string(LangStr);
        jniEnv->ReleaseStringUTFChars(jstrLang, LangStr);

        // кешируем директорию для записи
        jstring jstrWriteDir = (jstring)jniEnv->CallStaticObjectMethod(AndroidWrapperState.UtilsClass, GetInternalWriteDir);
        const char* javaWriteDir = jniEnv->GetStringUTFChars(jstrWriteDir, 0);
        AndroidWrapperState.sInternalWriteDir = std::string(javaWriteDir);
        jniEnv->ReleaseStringUTFChars(jstrWriteDir, javaWriteDir);
        if (AndroidWrapperState.sInternalWriteDir.back() != '/') {
            AndroidWrapperState.sInternalWriteDir += "/";
        }

        // кешируем внутреннюю директорию
        //jstring jstrDir = (jstring)jniEnv->CallStaticObjectMethod(AndroidWrapperState.UtilsClass, GetInternalDir);
        //const char* javaDir = jniEnv->GetStringUTFChars(jstrDir, 0);
        //AndroidWrapperState.sInternalDir = std::string(javaDir);
        //jniEnv->ReleaseStringUTFChars(jstrDir, javaDir);
        //if(AndroidWrapperState.sInternalDir.back() != '/') {
        //    AndroidWrapperState.sInternalDir += "/";
        //}

        jint value = jniEnv->CallStaticIntMethod(AndroidWrapperState.UtilsClass, GetApiLevel);
        AndroidWrapperState.sApiLevel = static_cast<int>(value);

        InitAssetsManager();
    }

    std::string getInternalWriteDir() {
        return AndroidWrapperState.sInternalWriteDir;
    }

    std::string getLanguage() {
        return AndroidWrapperState.sLanguage;
    }

    int getApiLevel() {
        return AndroidWrapperState.sApiLevel;
    }

    bool dirRemove(const char* Dir) {
        return private_DirRemove(Dir, false);
    }

    bool dirRemoveRecursive(const char* Dir) {
        return private_DirRemove(Dir, true);
    };

    bool dirCreate(const char* Dir) {
        if (!AndroidWrapperState.midMkDir)
        {
            logError("AndroidWrapper MkDir Java method not Found");
            return false;
        }

        AndroidJavaUTF8String url_jstring(jniEnv, Dir);
        jint value = jniEnv->CallStaticIntMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midMkDir, url_jstring.get());
        int retval = (int)value;

        switch (retval)
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
    }

    void messageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, uint32_t TimeOutMS) {
        if (!AndroidWrapperState.midShowMessageBox)
        {
            logError("AndroidWrapper showMessageBox Java method not Found");
            return;
        }

        AndroidJavaUTF8String jstring_Title(jniEnv, Title);
        AndroidJavaUTF8String jstring_Message(jniEnv, Message);
        AndroidJavaUTF8String jstring_Button1(jniEnv, Button1);
        AndroidJavaUTF8String jstring_Button2(jniEnv, Button2);
        AndroidJavaUTF8String jstring_Button3(jniEnv, Button3);

        uint64_t tm = static_cast<uint64_t>(TimeOutMS);

        jniEnv->CallStaticVoidMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midShowMessageBox, Code, jstring_Title.get(), jstring_Message.get(), jstring_Button1.get(), jstring_Button2.get(), jstring_Button3.get(), tm);
    }

    void shareText(const char* Title, const char* Message) {
        if (!AndroidWrapperState.midShareText) {
            logError("AndroidWrapper midShareText Java method not Found");
            return;
        }

        AndroidJavaUTF8String jstring_Title(jniEnv, Title);
        AndroidJavaUTF8String jstring_Message(jniEnv, Message);
        jniEnv->CallStaticVoidMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midShareText, jstring_Title.get(), jstring_Message.get());
    }

    int getAudioOutputRate() {
        if (!AndroidWrapperState.midGetAudioOutputRate) {
            logError("AndroidWrapper getAudioOutputRate Java method not Found");
            return -1; // return default value
        }
        jint value = jniEnv->CallStaticIntMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midGetAudioOutputRate);
        //logDebug("outputRate %d", static_cast<int>(value));
        return static_cast<int>(value);
    };

    int getAudioOutputBufferSize() {
        if (!AndroidWrapperState.midGetAudioOutputBufferSize) {
            logError("AndroidWrapper getAudioOutputBufferSize Java method not Found");
            return -1; // return default value
        }
        jint value = jniEnv->CallStaticIntMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midGetAudioOutputBufferSize);
        //logDebug("outputBufferSize %d", static_cast<int>(value));
        return static_cast<int>(value);
    };

    bool launchAppReviewIfAvailable() {
        if (!AndroidWrapperState.midLaunchAppReviewIfAvailable) {
            logError("AndroidWrapper LaunchAppReviewIfAvailable Java method not Found");
            return false;
        }
        jniEnv->CallStaticVoidMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midLaunchAppReviewIfAvailable);
        return true;
    }

    bool requestFlexibleUpdateIfAvailable() {
        if (!AndroidWrapperState.midRequestFlexibleUpdateIfAvailable) {
            logError("AndroidWrapper RequestFlexibleUpdateIfAvailable Java method not Found");
            return false;
        }
        jniEnv->CallStaticVoidMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midRequestFlexibleUpdateIfAvailable);
        return true;
    }

    std::string getAppVersionCode() {
        if (!AndroidWrapperState.midGetAppVersionCode) {
            logError("AndroidWrapper GetAppVersionCode Java method not Found");
            return "Unknown";
        }

        jstring jstrVersion = (jstring)jniEnv->CallStaticObjectMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midGetAppVersionCode);
        const char* VersionStr = jniEnv->GetStringUTFChars(jstrVersion, 0);
        const std::string versionString(VersionStr);
        jniEnv->ReleaseStringUTFChars(jstrVersion, VersionStr);
        return versionString;
    }

    std::string getAppVersionName() {
        if (!AndroidWrapperState.midGetAppVersionName) {
            logError("AndroidWrapper GetAppVersionName Java method not Found");
            return "Unknown";
        }

        jstring jstrVersion = (jstring)jniEnv->CallStaticObjectMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midGetAppVersionName);
        const char* VersionStr = jniEnv->GetStringUTFChars(jstrVersion, 0);
        const std::string versionString(VersionStr);
        jniEnv->ReleaseStringUTFChars(jstrVersion, VersionStr);
        return versionString;
    }

    bool showToast(const char* Message, int Duration, int Gravity, int xOffset, int yOffset) {
        if (!AndroidWrapperState.midShowToast) {
            logError("AndroidWrapper::ShowToast Java method not Found");
            return false;
        }
        AndroidJavaUTF8String url_jstring(jniEnv, Message);
        //logDebug("Call Method");
        jniEnv->CallStaticVoidMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midShowToast, url_jstring.get(), Duration, Gravity, xOffset, yOffset);
        return true;
    }

    std::unique_ptr<std::istream> getAssetStream(const char* FileName) {
        class asset_streambuf : public std::streambuf {
        public:
            asset_streambuf(AAsset* the_asset)
                : the_asset_(the_asset) {
                char* begin = (char*)AAsset_getBuffer(the_asset);
                char* end = begin + AAsset_getLength64(the_asset);
                setg(begin, begin, end);
            }
            ~asset_streambuf() {
                AAsset_close(the_asset_);
            }

            asset_streambuf(const asset_streambuf& rhs) = delete; // Копирующий: конструктор
            asset_streambuf& operator= (const asset_streambuf& rhs) = delete; // Оператор копирующего присваивания
            asset_streambuf& operator= (asset_streambuf&& rhs) = delete; // Оператор перемещающего присваивания
            asset_streambuf(asset_streambuf&& rhs) = delete; // Перемещающий: конструктор
        private:
            AAsset* the_asset_;
        };

        class asset_stream : public std::istream {
        public:
            asset_stream(AAsset* the_asset) : std::istream(&sb), sb(the_asset) {}
        private:
            asset_streambuf sb;
        };

        auto asset = openAsset(FileName);
        if (asset) {
            return std::make_unique<asset_stream>(asset);
        }
        return nullptr;
    }

    AAsset* openAsset(const char* FileName) {
        if (AndroidWrapperState.AssetMgr) {
            auto asset = AAssetManager_open(AndroidWrapperState.AssetMgr, FileName, AASSET_MODE_BUFFER);
            if (!asset) {
                logError("Asset not found %s", FileName);
            }
            return asset;
        }
        else {
            logError("AndroidWrapperState.AssetMgr not initialized");
        }
        return nullptr;
    }

    bool openURL(const char* url) {
        if (!AndroidWrapperState.midOpenURL) {
            logError("AndroidWrapper::openURL Java method not Found");
            return false;
        }
        AndroidJavaUTF8String url_jstring(jniEnv, url);
        jniEnv->CallStaticVoidMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midOpenURL, url_jstring.get());
        return true;
    }

    static void onMessageboxCallback(int Code, int Result);
private:
    struct {
        AAssetManager* AssetMgr{ nullptr };
        jclass    UtilsClass{ nullptr };
        jmethodID midDirectoryDelete{ nullptr };
        jmethodID midShowToast{ nullptr };
        jmethodID midMkDir{ nullptr };
        jmethodID midShowMessageBox{ nullptr };
        jmethodID midGetAssetManager{ nullptr };
        jmethodID midShareText{ nullptr };
        //jmethodID midSharePNG                 { nullptr };
        jmethodID midGetAudioOutputRate{ nullptr };
        jmethodID midGetAudioOutputBufferSize{ nullptr };
        jmethodID midLaunchAppReviewIfAvailable{ nullptr };
        jmethodID midRequestFlexibleUpdateIfAvailable{ nullptr };
        jmethodID midGetAppVersionCode{ nullptr };
        jmethodID midGetAppVersionName{ nullptr };
        jmethodID midOpenURL{ nullptr };

        std::string sLanguage;
        //std::string sInternalDir;
        std::string sInternalWriteDir;
        int         sApiLevel;
    } AndroidWrapperState;

    jclass getJavaClass(JNIEnv* Env, const char* ClassName, bool PrintTraceOnError) {
        jclass localClass = Env->FindClass(ClassName);
        if (Env->ExceptionCheck()) {
            logWarning("Java class %s not found", ClassName);
            if (PrintTraceOnError) {
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
            if (PrintTraceOnError) {
                Env->ExceptionDescribe();
            }
            Env->ExceptionClear();
            return nullptr;
        }
        return method;
    }

    void InitAssetsManager() {
        if (!AndroidWrapperState.AssetMgr) {
            if (!AndroidWrapperState.midGetAssetManager) {
                logError("AndroidWrapper GetAssetManager Java method not Found");
                return;
            }

            jobject jAssetsMgr = (jobject)jniEnv->CallStaticObjectMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midGetAssetManager);
            AndroidWrapperState.AssetMgr = AAssetManager_fromJava(jniEnv, jAssetsMgr);
        }
    }

    bool private_DirRemove(const char* Dir, bool Recursive) {
        if (!AndroidWrapperState.midDirectoryDelete) {
            logError("AndroidWrapper: DirectoryDelete Java method not Found");
            return false;
        }

        AndroidJavaUTF8String url_jstring(jniEnv, Dir);
        int Recurs = Recursive ? 1 : 0;
        jint value = jniEnv->CallStaticIntMethod(AndroidWrapperState.UtilsClass, AndroidWrapperState.midDirectoryDelete, url_jstring.get(), Recurs);
        int result = value;

        return ((result == 0) ? (true) : (false));
    }


    static MessageBoxCallback* callback;
    JNIEnv* jniEnv{ nullptr };
};

#endif