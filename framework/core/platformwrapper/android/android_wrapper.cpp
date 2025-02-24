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

    virtual bool vInit() override {
        wrapper = std::make_unique<AndroidUtilsWrapper>((JNIEnv*)SDL_AndroidGetJNIEnv(), this);
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

    virtual void vMessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, Uint32 TimeOutMS) override {
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