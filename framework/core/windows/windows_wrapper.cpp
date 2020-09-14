#include <windows.h>
#include <shellapi.h>
#include "../platforms.h"
#include "customevents.h"

class WinDesktopWrapper : public PlatformWrapper {
    bool vInit() override {
        return true;
    };
    Locale::Lang vGetDeviceLanguage() override {
        return Locale::Lang::Russian;
    };

    std::string vGetEnvVariable(const char* Variable) override {
        char buffer[1024];
        GetEnvironmentVariable((LPCTSTR)Variable, buffer, sizeof(buffer));
        return std::string(buffer);
    };

    std::string vGetInternalWriteDir() override {
        return "data-ram/";
    };

    bool vDirCreate(const char* Dir) override {
        std::string path(Dir);
        decltype(path.find_first_of("\\/", 1)) pos = 0;
        do
        {
            pos = path.find_first_of("\\/", pos + 1);
            if (!CreateDirectory((LPCTSTR)path.substr(0, pos).c_str(), nullptr))
            {
                if (GetLastError() != ERROR_ALREADY_EXISTS)
                {
                    logError("Create dir error <%s> <%s>", Dir, path.substr(0, pos).c_str());
                    return false;
                }
            }
        } while (pos != std::string::npos);
        return true;
    };

    bool vDirExists(const char* Dir) override {
        DWORD dwAttrib = GetFileAttributes((LPCTSTR)Dir);
        return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
    };

    bool vDirRemove(const char* Dir) override {
        if (RemoveDirectory((LPCTSTR)Dir)) {
            return true;
        }

        return false;
    };

    bool vDirRemoveRecursive(const char* Dir) override {
        WIN32_FIND_DATA fd;
        std::string Path = std::string(Dir) + "/";

        std::string localPath;
        auto Handle = FindFirstFile((LPCTSTR)std::string(Path + "*").c_str(), &fd);

        if (INVALID_HANDLE_VALUE != Handle) {
            do {
                localPath = std::string(fd.cFileName);
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && (localPath == ".." || localPath == "."))) {
                    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) // this a dir
                    {
                        DirRemoveRecursive(std::string(Path + localPath).c_str());
                    }
                    else // file
                    {
                        DeleteFile((LPCTSTR)std::string(Path + localPath).c_str());
                    }
                }
            } while (FindNextFile(Handle, &fd) != 0);
        }
        FindClose(Handle);
        RemoveDirectory((LPCTSTR)Dir);
        return !vDirExists(Dir);
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
    ;

    // Activity functions
    bool vOpenURL(const char* url)  override {
        ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
        return true;
    };

    void vMessageBoxShow(int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, Uint32 TimeOutMS)  override {
        SDL_MessageBoxButtonData buttons[3];
        int buttonCnt = 1;

        buttons[0].buttonid = 0;
        buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
        buttons[0].text = Button1;

        if (Button2 != nullptr && Button2[0] != '\0')
        {
            ++buttonCnt;
            buttons[1].buttonid = 1;
            buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
            buttons[1].text = Button2;
        }

        if (Button3 != nullptr && Button3[0] != '\0')
        {
            ++buttonCnt;
            buttons[2].buttonid = 2;
            buttons[2].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
            buttons[2].text = Button3;

            buttons[1].flags = 0;
        }

        const SDL_MessageBoxColorScheme colorScheme = {
            { /* .colors (.r, .g, .b) */
                /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
                { 88, 135, 63 },
                /* [SDL_MESSAGEBOX_COLOR_TEXT] */
                { 250, 250, 250 },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
                { 255, 255, 0 },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
                { 0, 0, 255 },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
                { 255, 0, 255 }
            }
        };

        const SDL_MessageBoxData messageboxdata = {
            SDL_MESSAGEBOX_INFORMATION, /* .flags */
            //NULL, /* .window */
            //CurrentContext.CurrentWindow,
            BWrapper::GetActiveWindow(),
            //NULL,
            Title, /* .title */
            Message, /* .message */
            buttonCnt, /* .numbuttons */
            buttons, /* .buttons */
            &colorScheme /* .colorScheme */
        };

        int buttonid;

        if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
            logError("error displaying message box");
            return;
        }

        if (buttonid == -1)
        {
            logDebug("no selection");
            CustomEvents::MessageBoxCallback(Code, 0); // 0 - Cancel
        }
        else
        {
            CustomEvents::MessageBoxCallback(Code, buttonid + 1); // msgBox::Action Button[n]
        }
    };
};

static WinDesktopWrapper winDesktopWrapper;

PlatformWrapper& PlatformWrapper::vGetInstance() {
    return winDesktopWrapper;
}