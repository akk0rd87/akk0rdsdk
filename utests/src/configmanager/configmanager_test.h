#ifndef __AKK0RD_SDK_CONFIG_MANAGER_TEST_H__
#define __AKK0RD_SDK_CONFIG_MANAGER_TEST_H__

#include <filesystem>
#include "gtest/gtest.h"
#include "configmanager.h"

TEST(TestSDK, ConfigManager) {
    const char* str1Key = "str1Key"; const char* str1Val = "str1Value";
    const char* str2Key = "str2Key"; const char* str2Val = "str2Value";
    const char* str3Key = "str3Key";

    const char* int1Key = "num1Key"; const int int1Val = 100500;
    const char* int2Key = "num2Key"; const int int2Val = 265;
    const char* int3Key = "num3Key";

    const char* baseDir = "tempdir1";
    const auto fileName = std::string(baseDir) + "/tempdir2/configfile.cfg";

    if(std::filesystem::exists(fileName)) {
        std::filesystem::remove(fileName);
    }

    if(std::filesystem::exists(baseDir)) {
        std::filesystem::remove_all(baseDir);
    }

    EXPECT_FALSE(std::filesystem::exists(baseDir));
    EXPECT_FALSE(std::filesystem::exists(fileName));

    auto checkValues = [&](ConfigManager& cfg){
        EXPECT_EQ(std::string(str1Val), cfg.GetStrValue(str1Key, "empty1"));
        EXPECT_EQ(std::string(str2Val), cfg.GetStrValue(str2Key, "empty2"));
        EXPECT_EQ(std::string("empty3"), cfg.GetStrValue(str3Key, "empty3"));

        EXPECT_EQ(int1Val, cfg.GetIntValue(int1Key, 10));
        EXPECT_EQ(int2Val, cfg.GetIntValue(int2Key, 20));
        EXPECT_EQ(30, cfg.GetIntValue(int3Key, 30));
    };

    {
        ConfigManager configManager;
        configManager.SetFile(fileName);
        configManager.SetStrValue(str1Key, str1Val);
        configManager.SetStrValue(str2Key, str2Val);
        configManager.SetIntValue(int1Key, int1Val);
        configManager.SetIntValue(int2Key, int2Val);

        checkValues(configManager);
        EXPECT_FALSE(std::filesystem::exists(baseDir));
        EXPECT_FALSE(std::filesystem::exists(fileName));
        configManager.Save();
        EXPECT_TRUE(std::filesystem::exists(baseDir));
        EXPECT_TRUE(std::filesystem::exists(fileName));
        checkValues(configManager);
        configManager.Load();
        checkValues(configManager);
    }

    {
        ConfigManager configManager;
        configManager.SetFile(fileName);
        configManager.Load();
        checkValues(configManager);
        configManager.Save();
        checkValues(configManager);
    }
}

#endif // __AKK0RD_SDK_CONFIG_MANAGER_TEST_H__