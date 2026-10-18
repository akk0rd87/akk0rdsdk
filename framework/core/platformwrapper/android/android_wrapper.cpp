#include <dirent.h>
#include <algorithm>
#include <mutex>
#include "android_wrapper.h"
#include "../platforms.h"
#include "customevents.h"
#include "basewrapper.h"
#include "android_utils_wrapper.h"

// Отступы безопасной области в пикселях surface; пишутся из UI-потока Android, читаются из потока SDL
struct SafeAreaInsets {
    int left{ 0 }, top{ 0 }, right{ 0 }, bottom{ 0 };
};
static std::mutex     safeAreaInsetsMutex;
static SafeAreaInsets safeAreaInsets;

extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_AkkordActivity_nativeSafeAreaInsetsChanged(JNIEnv*, jclass, jint Left, jint Top, jint Right, jint Bottom)
    {
        {
            const std::lock_guard<std::mutex> lock(safeAreaInsetsMutex);
            safeAreaInsets.left = Left;
            safeAreaInsets.top = Top;
            safeAreaInsets.right = Right;
            safeAreaInsets.bottom = Bottom;
        }

        // Размер окна при этом может не меняться, поэтому будим цикл событий приложения сами
        if (auto window = BWrapper::GetActiveWindow()) {
            SDL_Event sdl_Event;
            SDL_zero(sdl_Event);
            sdl_Event.type = SDL_WINDOWEVENT;
            sdl_Event.window.windowID = SDL_GetWindowID(window);
            sdl_Event.window.event = SDL_WINDOWEVENT_EXPOSED;
            SDL_PushEvent(&sdl_Event);
        }
    };
}

class AndroidPlatformWrapper : public PlatformWrapper, public MessageBoxCallback {
    std::unique_ptr<AndroidUtilsWrapper> wrapper;

    // MessageBoxCallback
    virtual void onResult(int Code, int Result) override {
        CustomEvents::MessageBoxCallback(Code, Result);
    }

    virtual bool vInit() override {
        wrapper = std::make_unique<AndroidUtilsWrapper>(AndroidWrapper::GetJNIEnv(), this);
        return wrapper ? true : false;
    };

    virtual Locale::Lang             vGetDeviceLanguage() override {
        return Locale::DecodeLang_ISO639_Code(wrapper->getLanguage().c_str());
    };

    virtual std::string              vGetInternalWriteDir() override {
        return wrapper->getInternalWriteDir();
    };

    virtual std::string              vGetInternalAssetsDir() override {
        return "";
    };

    virtual bool vDirCreate(const char* Dir) override {
        return wrapper->dirCreate(Dir);
    };

    virtual bool vDirExists(const char* Dir) override {
        DIR* dir = opendir(Dir);
        if (dir != nullptr) {
            closedir(dir);
            return true;
        }
        return false;
    };

    virtual bool vDirRemove(const char* Dir) override {
        return wrapper->dirRemove(Dir);
    };

    virtual bool vDirRemoveRecursive(const char* Dir) override {
        return wrapper->dirRemoveRecursive(Dir);
    };

    virtual bool vGetDirContent(const char* Dir, DirContentElementArray& ArrayList)  override {
        // https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
        struct dirent* entry;
        DIR* dir = opendir(Dir);

        if (dir != nullptr) {
            while ((entry = readdir(dir)) != nullptr) {
                auto Name = std::string(entry->d_name);
                if ((Name != "." && Name != "..") || (DT_DIR != entry->d_type)) {
                    ArrayList.emplace_back(std::make_unique<DirContentElement>());
                    ArrayList.back()->Name = Name;
                    ArrayList.back()->isDir = (DT_DIR == entry->d_type ? 1 : 0);
                }
            }
            closedir(dir);
        }

        return true;
    };

    virtual void vMessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, const char* Button4, Uint32 TimeOutMS) override {
        // Button4 пока не поддержан на этом (SDL/JNI) Android-бэкенде — сохраняем существующее поведение с 3 кнопками.
        wrapper->messageBoxShow(Code, Title, Message, Button1, Button2, Button3, TimeOutMS);
    };

    virtual void vShareText(const char* Title, const char* Message) override {
        wrapper->shareText(Title, Message);
    };

    virtual int vGetAudioOutputRate() override {
        return wrapper->getAudioOutputRate();
    };

    virtual int vGetAudioOutputBufferSize() override {
        return wrapper->getAudioOutputBufferSize();
    };

    virtual bool vLaunchAppReviewIfAvailable() override {
        return wrapper->launchAppReviewIfAvailable();
    }

    virtual bool vRequestFlexibleUpdateIfAvailable() override {
        return wrapper->requestFlexibleUpdateIfAvailable();
    }

    virtual std::string vGetAppVersionCode() override {
        return wrapper->getAppVersionCode();
    }

    virtual std::string vGetAppVersionName() override {
        return wrapper->getAppVersionName();
    }

    virtual bool vOpenURL(const char* url) override {
        return wrapper->openURL(url);
    }

    virtual void vSetSystemBarsAppearance(bool LightStatusBar, bool LightNavigationBar) override {
        wrapper->setSystemBarsAppearance(LightStatusBar, LightNavigationBar);
    }

    virtual AkkordRect vGetWindowSafeArea(AkkordRenderer* CurrentRenderer, AkkordWindow* CurrentWindow) override {
        SafeAreaInsets insets;
        {
            const std::lock_guard<std::mutex> lock(safeAreaInsetsMutex);
            insets = safeAreaInsets;
        }

        // Размер берём из рендерера — те же пиксели, что возвращает BWrapper::GetScreenSize()
        int rendererW = 0, rendererH = 0;
        SDL_GetRendererOutputSize(CurrentRenderer, &rendererW, &rendererH);

        // при повороте размер рендерера и insets могут на кадр разойтись
        const int w = rendererW - insets.left - insets.right;
        const int h = rendererH - insets.top - insets.bottom;
        if (w <= 0 || h <= 0) {
            return AkkordRect(0, 0, rendererW, rendererH);
        }
        return AkkordRect(insets.left, insets.top, w, h);
    }

public:
    int GetApiLevel() {
        return wrapper->getApiLevel();
    };

    bool ShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset) {
        return wrapper->showToast(Message, static_cast<int>(Duration), Gravity, xOffset, yOffset);
    }

    std::unique_ptr<FileBuffer> GetAssetBuf(const char* FileName) {
        class localBuffer : public FileBuffer {
        public:
            localBuffer(AAsset* the_asset)
                : the_asset_(the_asset) {
                begin = (char*)AAsset_getBuffer(the_asset);
                end = begin + AAsset_getLength64(the_asset);
            }
            ~localBuffer() {
                AAsset_close(the_asset_);
            }
            virtual char* Begin() override { return begin; };
            virtual char* End() override { return end; };
        private:
            AAsset* the_asset_;
            char* begin;
            char* end;
        };

        auto asset = wrapper->openAsset(FileName);
        if (asset) {
            return std::make_unique<localBuffer>(asset);
        }
        return nullptr;
    }

    std::unique_ptr<std::istream> vGetAssetStream(const char* FileName) override {
        return wrapper->getAssetStream(FileName);
    };

    std::unique_ptr<FileBuffer> vGetFileBuf(const char* FileName, BWrapper::FileSearchPriority SearchPriority) override {
        if (BWrapper::FileSearchPriority::Assets == SearchPriority) {
            return GetAssetBuf(FileName);
        }
        else {
            return GetFileBuf(FileName);
        }
    }
};

static AndroidPlatformWrapper androidPlatformWrapper;
PlatformWrapper& PlatformWrapper::vGetInstance() {
    return androidPlatformWrapper;
};

bool AndroidWrapper::AndroidShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset) {
    return androidPlatformWrapper.ShowToast(Message, Duration, Gravity, xOffset, yOffset);
};

int AndroidWrapper::AndroidGetApiLevel() {
    return androidPlatformWrapper.GetApiLevel();
};

JNIEnv* AndroidWrapper::GetJNIEnv() {
    return (JNIEnv*)SDL_AndroidGetJNIEnv();
};

jobject AndroidWrapper::GetActivity() {
    return reinterpret_cast<jobject>(SDL_AndroidGetActivity());
};

bool AndroidWrapper::CallActivityBooleanMethod(const char* methodName) {
    auto env = GetJNIEnv();
    auto Activity = GetActivity();
    jclass clazz(env->GetObjectClass(Activity));
    jmethodID javaMethod = env->GetMethodID(clazz, methodName, "()Z");
    if (nullptr == javaMethod) {
        env->DeleteLocalRef(Activity);
        env->DeleteLocalRef(clazz);
        return false;
    }
    jboolean result = env->CallBooleanMethod(Activity, javaMethod);
    env->DeleteLocalRef(Activity);
    env->DeleteLocalRef(clazz);
    logDebug("%s %d", methodName, result == JNI_TRUE);
    return result == JNI_TRUE;
}