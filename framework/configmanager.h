#pragma once
#ifndef __AKK0RD_SDK_CONFIG_MANAGER_H__
#define __AKK0RD_SDK_CONFIG_MANAGER_H__

#include <unordered_map>
#include "basewrapper.h"

// Класс управления настройками в виде списка Key-Value
class ConfigManager
{
public:
    bool         Load();
    bool         Save();
    void         Clear() { ConfigList.clear(); };

    void         SetFile(const std::string& FileName, BWrapper::FileSearchPriority SearchPriority) { this->FileName = FileName; this->FSearchPriority = SearchPriority; };

    bool         SetIntValue(const char* Key, int         Value) { SetValue(Key, std::to_string(Value).c_str()); return true; };
    bool         SetStrValue(const char* Key, const char* Value) { SetValue(Key, Value); return true; };

    int          GetIntValue(const char* Key, int         DefaultValue) { const auto it = GetConfigIt(Key); return (it != ConfigList.end() ? std::stoi(it->second) : DefaultValue); };
    std::string  GetStrValue(const char* Key, const char* DefaultValue) { const auto it = GetConfigIt(Key); return (it != ConfigList.end() ? it->second : DefaultValue); };

    void         PrintConfig();

    ConfigManager() {};
    ~ConfigManager() {};
private:
    // список пар Key-Value
    std::unordered_map<std::string, std::string> ConfigList;
    void SetValue(const char* Key, const char* Value);

    BWrapper::FileSearchPriority FSearchPriority = BWrapper::FileSearchPriority::FileSystem;
    std::string FileName;

    ConfigManager(const ConfigManager& rhs) = delete; // Копирующий: конструктор
    ConfigManager(ConfigManager&& rhs) = default; // Перемещающий: конструктор
    ConfigManager& operator= (const ConfigManager& rhs) = delete; // Оператор копирующего присваивания
    ConfigManager& operator= (ConfigManager&& rhs) = default; // Оператор перемещающего присваивания

    decltype(ConfigList)::iterator GetConfigIt(const char* Key) {
        return ConfigList.find(Key);
    };
};

#endif // __AKK0RD_SDK_CONFIG_MANAGER_H__
