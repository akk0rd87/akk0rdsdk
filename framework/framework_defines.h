#pragma once
#ifndef __AKK0RD_PUBLIC_DEFINES__
#define __AKK0RD_PUBLIC_DEFINES__

#include "SDL.h"
#include <limits>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>

// Макросы дебага
#if defined(_DEBUG) && defined(__WIN32__) || defined(__ANDROID__) && !defined(NDEBUG) || defined(__APPLE__) && defined(__DEBUG__) || defined(__CODEBLOCKS)
	#define __AKK0RD_DEBUG_MACRO__ 1
#endif

#define VALUE_BETWEEN(Val, Begin, End) (Begin <= Val && Val <= End ? true : false)

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

#ifdef __AKK0RD_DEBUG_MACRO__
	#define log(LogPriority, fmt, ...)  BWrapper::Log(LogPriority, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
	#define log(LogPriority, fmt, ...)
#endif

#define logVerbose(fmt, ...)        log(BWrapper::LogPriority::Verbose , fmt, ##__VA_ARGS__)
#define logDebug(fmt, ...)          log(BWrapper::LogPriority::Debug   , fmt, ##__VA_ARGS__)
#define logInfo(fmt, ...)           log(BWrapper::LogPriority::Info    , fmt, ##__VA_ARGS__)
#define logWarning(fmt, ...)        log(BWrapper::LogPriority::Warning , fmt, ##__VA_ARGS__)
#define logError(fmt, ...)          log(BWrapper::LogPriority::Error   , fmt, ##__VA_ARGS__)
#define logCritical(fmt, ...)       log(BWrapper::LogPriority::Critical, fmt, ##__VA_ARGS__)

struct LogParamsStruct
{
    bool showFile, showFunction, showLine;
    unsigned lenFile, lenFunction, lenLine;
    LogParamsStruct()
    {
        showFile = showLine = showFunction = true;
        lenFile = lenFunction = 30;
        lenLine = 6;
    };
};

#endif // __AKK0RD_PUBLIC_DEFINES__
