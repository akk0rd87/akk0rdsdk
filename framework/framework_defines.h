#pragma once
#ifndef __AKK0RD_PUBLIC_DEFINES__
#define __AKK0RD_PUBLIC_DEFINES__

#include "SDL.h"
#include <limits>

#define GREATEST(A, B) (A > B ? A : B)
#define LEAST(A, B) (A < B ? A : B)
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

typedef std::vector<DirContentElement*> DirContentElementArray;

class GConstants
{
public:
    static unsigned unsigned_max()
    {
        return (std::numeric_limits<unsigned>::max)();
    };

    static int int_min()
    {
        return (std::numeric_limits<int>::min)();
    };
    
    static int int_max()
    {
        return (std::numeric_limits<int>::max)();
    };
};

#define log(LogPriority, fmt, ...)  BWrapper::Log(LogPriority, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
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