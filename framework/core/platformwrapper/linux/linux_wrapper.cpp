#include "../platforms.h"

class LinuxDesktopWrapper : public PlatformWrapper {
    std::string vGetEnvVariable(const char* Variable) override {
        return "";
    };

    bool vGetDirContent(const char* Dir, DirContentElementArray& ArrayList)  override {
        return true;
    };
};

PlatformWrapper& PlatformWrapper::vGetInstance() {
    static LinuxDesktopWrapper linuxDesktopWrapper;
    return linuxDesktopWrapper;
}