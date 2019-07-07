#pragma once
#ifndef __AKK0RD_CONFIG_MANAGER_H__
#define __AKK0RD_CONFIG_MANAGER_H__

#include "basewrapper.h"

// Класс управления настройками в виде списка Key-Value
class ConfigManager
{
public:
    bool         Load();
    bool         Save();
    void         Clear();

    bool         SetFile    (const char* FileName, BWrapper::FileSearchPriority SearchPriority);

    bool         SetIntValue(const char* Key, int         Value);
    bool         SetStrValue(const char* Key, const char* Value);

    int          GetIntValue(const char* Key, int         DefaultValue);
    std::string  GetStrValue(const char* Key, const char* DefaultValue);

    void         PrintConfig();

    ConfigManager();
    ~ConfigManager();

private:
    class ConfigStruct
    {
    public:
        std::string Key;
        std::string Value;
        ~ConfigStruct()
        {
            Key.clear();
            Value.clear();
        }
    };

    // список пар Key-Value
    std::vector<ConfigStruct> ConfigVector;

    unsigned GetConfigIndex(const char* Key);
    void SetValue(const char* Key, const char* Value);

    BWrapper::FileSearchPriority FSearchPriority = BWrapper::FileSearchPriority::FileSystem;
    std::string FileName;
    void Destroy();

    //Запрещаем создавать экземпляр класса ConfigManager
    ConfigManager(ConfigManager& rhs) = delete; // Копирующий: конструктор
    ConfigManager(ConfigManager&& rhs) = delete; // Перемещающий: конструктор
    ConfigManager& operator= (ConfigManager&& rhs) = delete; // Оператор перемещающего присваивания
};


#endif // __AKK0RD_CONFIG_MANAGER_H__
