#pragma once
#ifndef __AKK0RD_WINDOWS_BASEWRAPPER_H__
#define __AKK0RD_WINDOWS_BASEWRAPPER_H__

#include <windows.h>
#include <shellapi.h>
#include "basewrapper.h"
#include "core/core_defines.h"

class WindowsWrapper
{
    public:
	static bool             OpenURL           (const char* url);
    //static BWrapper::Lang   GetLanguage       ();
    static std::string      GetEnvVariable    (const char* EnvVarName);
	//static bool             FileExists      (const char* FileName);
    static bool             DirectoryExists   (const char* Dir);
    static bool             DirCreate         (const char* Dir);
    static bool             DirRemove         (const char* Dir);
    static bool             DirRemoveRecursive(const char* Dir);
    //static bool             DirRename       (const char* Old, const char* New);

    static bool             GetDirContent  (const char* Dir, DirContentElementArray& ArrayList);
};


bool WindowsWrapper::OpenURL(const char* url)
{
    HINSTANCE res;
    //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    res = ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
    //CoUninitialize();
    return true;
}

/*
BWrapper::Lang WindowsWrapper::GetLanguage()
{
    auto language = GetUserDefaultUILanguage();
    return BWrapper::Lang::Russian;
}
*/


std::string WindowsWrapper::GetEnvVariable(const char* EnvVarName)
{
    char buffer[1024];
    GetEnvironmentVariable((LPCTSTR)EnvVarName, buffer, sizeof(buffer));

    return std::string(buffer);
}

bool WindowsWrapper::DirectoryExists(const char* Dir)
{
    DWORD dwAttrib = GetFileAttributes((LPCTSTR)Dir);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool WindowsWrapper::DirCreate(const char* Dir)
{
    // Recursive Directory Creation
    // https://stackoverflow.com/questions/1530760/how-do-i-recursively-create-a-folder-in-win32
    // https://stackoverflow.com/questions/1517685/recursive-createdirectory
    //return CreateDirectory((LPCTSTR)Dir, nullptr);

    // CreateDirectory https://msdn.microsoft.com/ru-ru/library/windows/desktop/aa363855(v=vs.85).aspx
    // GetLastError    https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms679360(v=vs.85).aspx

    std::string path(Dir);
    unsigned int pos = 0;
    do
    {
        pos = path.find_first_of("\\/", pos + 1);
        if(!CreateDirectory((LPCTSTR)path.substr(0, pos).c_str(), nullptr))
        {
            if(GetLastError() != ERROR_ALREADY_EXISTS)
            {
                logError("Create dir error <%s> <%s>", Dir, path.substr(0, pos).c_str());
                return false;
            }
        }
    } while (pos != std::string::npos);

    return true;
}


bool WindowsWrapper::GetDirContent(const char* Dir, DirContentElementArray& ArrayList)
{
    WIN32_FIND_DATA fd;
    std::string Path = std::string(Dir) + "/*";
    auto Handle = FindFirstFile((LPCTSTR)Path.c_str(), &fd);
    if (INVALID_HANDLE_VALUE != Handle)
    {
        do
        {
            Path = std::string(fd.cFileName);
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && (Path == ".." || Path == ".")))
            {
                auto dc = new DirContentElement();
                dc->Name = std::string(fd.cFileName);
                dc->isDir = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
                ArrayList.push_back(dc);
            }
        } while (FindNextFile(Handle, &fd) != 0);
    }
    FindClose(Handle);
    return true;
}


bool WindowsWrapper::DirRemoveRecursive(const char* Dir)
{
    WIN32_FIND_DATA fd;
    std::string Path = std::string(Dir) + "/";

    std::string localPath;
    auto Handle = FindFirstFile((LPCTSTR)std::string(Path + "*").c_str(), &fd);

    if (INVALID_HANDLE_VALUE != Handle)
    {
        do
        {
            localPath = std::string(fd.cFileName);
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && (localPath == ".." || localPath == ".")))
            {
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

    return !DirectoryExists(Dir);
};

bool WindowsWrapper::DirRemove(const char* Dir)
{
    if (RemoveDirectory((LPCTSTR)Dir))
        return true;

    return false;
}

#endif //__AKK0RD_WINDOWS_BASEWRAPPER_H__

// ${AKKORD_SDK_HOME}\framework\basewrapper\windows\windows_wrapper.cpp
