#pragma once
#ifndef __AKK0RD_SDK_PUBLIC_DEFINES__
#define __AKK0RD_SDK_PUBLIC_DEFINES__

#include "SDL.h"
#include <limits>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>

// Макросы дебага
#if defined(_DEBUG) && defined(__WINDOWS__) || defined(__ANDROID__) && !defined(NDEBUG) || defined(__APPLE__) && defined(__DEBUG__) || defined(__CODEBLOCKS)
#define __AKK0RD_SDK_DEBUG_MACRO__ 1
#endif

template <class T>
bool VALUE_BETWEEN(T Val, T Begin, T End) {
    return (Begin <= Val && Val <= End ? true : false);
}

typedef SDL_Window   AkkordWindow;
typedef SDL_Renderer AkkordRenderer;

struct membuf : std::streambuf
{
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};

struct DirContentElement
{
    std::string Name;
    bool isDir;
};

typedef std::vector<std::unique_ptr<DirContentElement>> DirContentElementArray;

#ifdef __AKK0RD_SDK_DEBUG_MACRO__
#define logVA(LogPriority, fmt, ...)  BWrapper::Log(LogPriority, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define logVA(LogPriority, fmt, ...)
#endif

#define logVerbose(fmt, ...)  logVA(BWrapper::LogPriority::Verbose , fmt, ##__VA_ARGS__)
#define logDebug(fmt, ...)    logVA(BWrapper::LogPriority::Debug   , fmt, ##__VA_ARGS__)
#define logInfo(fmt, ...)     logVA(BWrapper::LogPriority::Info    , fmt, ##__VA_ARGS__)
#define logWarning(fmt, ...)  logVA(BWrapper::LogPriority::Warning , fmt, ##__VA_ARGS__)
#define logError(fmt, ...)    logVA(BWrapper::LogPriority::Error   , fmt, ##__VA_ARGS__)
#define logCritical(fmt, ...) logVA(BWrapper::LogPriority::Critical, fmt, ##__VA_ARGS__)

struct LogParamsStruct
{
    bool showFile{ true }, showFunction{ true }, showLine{ true };
    unsigned lenFile{ 30 }, lenFunction{ 30 }, lenLine{ 6 };
};

#endif // __AKK0RD_SDK_PUBLIC_DEFINES__
