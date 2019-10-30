#ifndef __AKK0RD_TESTCONFIGMANAGER_H__
#define __AKK0RD_TESTCONFIGMANAGER_H__

#include <string>
#include "basewrapper.h"
#include "configmanager.h"
#include "ut_defines.h"

class ConfigNamagerTester {
public:
    void Run() {
        const char* int1Option = "int_VALUE_1"; const int int1Value = 100500;
        const char* int2Option = "int_VALUE_2"; const int int2Value = 13113131;
        const char* str1Option = "str_VALUE_1"; const char* str1Value = "Hello world again!!??";
        const char* str2Option = "str_VALUE_2"; const char* str2Value = "0";

        {
            ConfigManager cManager;
            cManager.SetFile(BWrapper::GetInternalWriteDir() + getConfigFilename(), BWrapper::FileSearchPriority::FileSystem);
            cManager.SetIntValue(int1Option, 600600);
            cManager.SetStrValue(str2Option, "str2Value");
            cManager.SetStrValue(str1Option, str1Value);
            cManager.SetIntValue(int2Option, int2Value);
            cManager.SetStrValue(str2Option, str2Value);
            cManager.SetIntValue(int1Option, int1Value);
            UT_CHECK(cManager.Save());
        }

        {
            FileReader fr;
            UT_CHECK(fr.Open((BWrapper::GetInternalWriteDir() + getConfigFilename()).c_str(), BWrapper::FileSearchPriority::FileSystem));
            std::string line;
            unsigned LineReaded {0}, CorrectLines {0};
            while (fr.ReadLine(line))
            {
                ++LineReaded;

                if(std::string(int1Option) + "=" + std::to_string(int1Value) == line)
                    ++CorrectLines;
                else if(std::string(int2Option) + "=" + std::to_string(int2Value) == line)
                    ++CorrectLines;
                else if(std::string(str1Option) + "=" + str1Value == line)
                    ++CorrectLines;
                else if(std::string(str2Option) + "=" + str2Value == line)
                    ++CorrectLines;
            };
            UT_CHECK(LineReaded == CorrectLines);
            UT_CHECK(LineReaded == 4);
            fr.Close();
        }

        {
            ConfigManager cManager;
            cManager.SetFile(BWrapper::GetInternalWriteDir() + getConfigFilename(), BWrapper::FileSearchPriority::FileSystem);
            UT_CHECK(cManager.Load());
            UT_CHECK(cManager.GetIntValue(int1Option, 1010) == int1Value);
            UT_CHECK(cManager.GetIntValue(int2Option, 1010) == int2Value);
            UT_CHECK(cManager.GetIntValue("unknown_value", 111010) == 111010);
            UT_CHECK(std::string(cManager.GetStrValue(str1Option, "1010e")) == std::string(str1Value));
            UT_CHECK(std::string(cManager.GetStrValue(str2Option, "1010z")) == std::string(str2Value));
        }

        UT_CHECK(BWrapper::DirRemoveRecursive(BWrapper::GetInternalWriteDir().c_str()));
    };

private:
    std::string getConfigFilename() { return "configfile.extension"; }
};

#endif // __AKK0RD_TESTCONFIGMANAGER_H__