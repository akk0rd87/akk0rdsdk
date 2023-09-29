#include <dirent.h>
#include "android_wrapper.h"
#include "../platforms.h"
#include "customevents.h"
#include "basewrapper.h"
#include "android_utils_wrapper.h"

class AndroidPlatformWrapper : public PlatformWrapper, public MessageBoxCallback {
    std::unique_ptr<AndroidUtilsWrapper> wrapper;

    // MessageBoxCallback
    virtual void onResult(int Code, int Result) override {
        CustomEvents::MessageBoxCallback(Code, Result);
    }

    bool vInit() override {
        wrapper = std::make_unique<AndroidUtilsWrapper>((JNIEnv*)SDL_AndroidGetJNIEnv(), this);
        return wrapper ? true : false;
    };

    Locale::Lang             vGetDeviceLanguage() override {
        return Locale::DecodeLang_ISO639_Code(wrapper->getLanguage().c_str());
    };

    std::string              vGetInternalWriteDir() override {
        return wrapper->getInternalWriteDir();
    };

    std::string              vGetInternalAssetsDir() override {
        return "";
    };

    bool vDirCreate(const char* Dir) override {
        return wrapper->dirCreate(Dir);
    };

    bool vDirExists(const char* Dir) override {
        DIR* dir = opendir(Dir);
        if (dir != nullptr) {
            closedir(dir);
            return true;
        }
        return false;
    };

    bool vDirRemove(const char* Dir) override {
        return wrapper->dirRemove(Dir);
    };

    bool vDirRemoveRecursive(const char* Dir) override {
        return wrapper->dirRemoveRecursive(Dir);
    };

    bool vGetDirContent(const char* Dir, DirContentElementArray& ArrayList)  override {
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

    void vMessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, Uint32 TimeOutMS) override {
        wrapper->messageBoxShow(Code, Title, Message, Button1, Button2, Button3, TimeOutMS);
    };

    void vShareText(const char* Title, const char* Message) override {
        wrapper->shareText(Title, Message);
    };

    int vGetAudioOutputRate() override {
        return wrapper->getAudioOutputRate();
    };

    int vGetAudioOutputBufferSize() override {
        return wrapper->getAudioOutputBufferSize();
    };

    bool vLaunchAppReviewIfAvailable() override {
        return wrapper->launchAppReviewIfAvailable();
    }

    bool vRequestFlexibleUpdateIfAvailable() override {
        return wrapper->requestFlexibleUpdateIfAvailable();
    }

    std::string vGetAppVersionCode() override {
        return wrapper->getAppVersionCode();
    }

    std::string vGetAppVersionName() override {
        return wrapper->getAppVersionName();
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