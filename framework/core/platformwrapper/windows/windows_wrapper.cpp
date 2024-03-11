#include <windows.h>
#include <shellapi.h>
#include "../platforms.h"

class WinDesktopWrapper : public PlatformWrapper {
    std::string vGetEnvVariable(const char* Variable) override {
        char buffer[1024];
        GetEnvironmentVariable((LPCTSTR)Variable, buffer, sizeof(buffer));
        return std::string(buffer);
    };

    bool vGetDirContent(const char* Dir, DirContentElementArray& ArrayList)  override {
        WIN32_FIND_DATA fd;
        auto Path = std::string(Dir) + "/*";
        auto Handle = FindFirstFile((LPCTSTR)Path.c_str(), &fd);
        if (INVALID_HANDLE_VALUE != Handle) {
            do {
                Path = std::string(fd.cFileName);
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && (Path == ".." || Path == "."))) {
                    ArrayList.emplace_back(std::make_unique<DirContentElement>());
                    ArrayList.back()->Name = Path;
                    ArrayList.back()->isDir = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
                }
            } while (FindNextFile(Handle, &fd) != 0);
        }
        FindClose(Handle);
        return true;
    };

};

PlatformWrapper& PlatformWrapper::vGetInstance() {
    static WinDesktopWrapper winDesktopWrapper;
    return winDesktopWrapper;
}