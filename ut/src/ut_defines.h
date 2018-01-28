#pragma once
#ifndef __AKK0RD_TEST_DEFINES_MANAGER_H__
#define __AKK0RD_TEST_DEFINES_MANAGER_H__

#include "basewrapper.h"

//#define UT_CHECK(BOOLVALUE) BWrapper::Log(BWrapper::LogPriority::Debug, "[%s] [%s] [%s] [%d]", (BOOLVALUE ? "OK" : "ERROR"), __FILE__, __FUNCTION__, __LINE__)

#define UT_CHECK(BOOLVALUE) statistic.Check(BOOLVALUE, __FILE__, __FUNCTION__, __LINE__)

class Statistic
{
    struct ErrorMsg
    {
        std::string File;
        std::string Function;
        unsigned Line;
    };

    std::vector<ErrorMsg*>ErrosVector;

    void Clear()
    {
        auto Size = ErrosVector.size();
        for (decltype(Size) i = 0; i < Size; i++)
            delete ErrosVector[i];

        ErrosVector.clear();
    }
public:
    void Check(bool BoolValue, const char* File, const char* Function, unsigned Line)
    {
        //BWrapper::Log(BWrapper::LogPriority::Debug, "[%s] [%s] [%s] [%d]", (BoolValue ? "OK" : "ERROR"), File, Function, Line);
        BWrapper::Log(BWrapper::LogPriority::Debug, File, Function, Line, "[%s]", (BoolValue ? "OK" : "ERROR"));

        if (!BoolValue)
        {
            auto err = new ErrorMsg();
            err->Function = std::string(Function);
            err->File = std::string(File);
            err->Line = Line;
            ErrosVector.push_back(err);
        }
    };

    void PrintResult()
    {
        logDebug("=======================");
        logDebug("======= RESULT ========");
        logDebug("=======================");
        logDebug("");
        
        auto Size = ErrosVector.size();
        if (Size)
        {
            logDebug("Errors %u", Size);
            for (decltype(Size) i = 0; i < Size; i++)
            {
                //BWrapper::Log(BWrapper::LogPriority::Debug, "[%s] [%s] [%s] [%d]", "ERROR", ErrosVector[i]->File.c_str(), ErrosVector[i]->Function.c_str(), ErrosVector[i]->Line);
                BWrapper::Log(BWrapper::LogPriority::Debug, ErrosVector[i]->File.c_str(), ErrosVector[i]->Function.c_str(), ErrosVector[i]->Line, "[%s]", "ERROR");
            }

            logDebug("");
            logDebug("FFFFFFF   AA     III  L      U     U RRRRR   EEEEEEE");
            logDebug("F        A  A     I   L      U     U R    R  E      ");
            logDebug("F       A    A    I   L      U     U R     R E      ");
            logDebug("F      A      A   I   L      U     U R     R E      ");
            logDebug("FFFF   A      A   I   L      U     U RRRRRR  EEEE   ");
            logDebug("F      AAAAAAAA   I   L      U     U R   R   E      ");
            logDebug("F      A      A   I   L      U     U R    R  E      ");
            logDebug("F      A      A   I   L       U   U  R     R E      ");
            logDebug("F      A      A  III  LLLLLLL  UUU   R     R EEEEEEE");
        }
        else
        {            
            logDebug("  SSSS   U     U   CCC     CCC   EEEEEEE   SSSS     SSSS  ");
            logDebug(" S    S  U     U  C   C   C   C  E        S    S   S    S ");
            logDebug("S        U     U C     C C     C E       S        S       ");
            logDebug(" S       U     U C       C       E        S        S      ");
            logDebug("  SSSS   U     U C       C       EEEE      SSSS     SSSS  ");
            logDebug("      S  U     U C       C       E             S        S ");
            logDebug("       S U     U C     C C     C E              S        S");
            logDebug(" S    S   U   U   C   C   C   C  E        S    S   S    S ");
            logDebug("  SSSS     UUU     CCC     CCC   EEEEEEE   SSSS     SSSS  ");
        }
    }    

    Statistic()
    {
        Clear();
    }

    ~Statistic()
    {        
        Clear();
    }
};

#endif // __AKK0RD_TEST_DEFINES_MANAGER_H__