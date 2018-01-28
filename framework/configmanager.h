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
        unsigned    KeyLength;
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
};

/////////////////////////////////////////////////
////////// REALIZATION
/////////////////////////////////////////////////

unsigned ConfigManager::GetConfigIndex(const char* Key)
{
    auto len = strlen(Key);

    for (decltype(ConfigVector.size()) i = 0; i < ConfigVector.size(); i++)
        if (len == ConfigVector[i].KeyLength)
            if (strcmp(Key, ConfigVector[i].Key.c_str()) == 0)
                return i;

    // return max unsigned value    
    return GConstants::unsigned_max();
}

void ConfigManager::SetValue(const char* Key, const char* Value)
{
    auto index = GetConfigIndex(Key);

    if (index < ConfigVector.size())
    {
        ConfigVector[index].Value = std::string(Value);
    }
    else
    {
        ConfigStruct str;
        ConfigVector.push_back(str);
        auto idx = ConfigVector.size() - 1;
        ConfigVector[idx].Key = std::string(Key);
        ConfigVector[idx].KeyLength = strlen(Key);
        ConfigVector[idx].Value = std::string(Value);
    }
}

bool ConfigManager::SetFile(const char* FileName, BWrapper::FileSearchPriority SearchPriority)
{
    this->FileName = std::string(FileName);
    this->FSearchPriority = SearchPriority;

    return true;
}

bool ConfigManager::Load()
{
    this->Destroy();

    ConfigStruct str;

    FileReader fr;
    if (fr.Open(FileName.c_str(), FSearchPriority))
    {
        std::string line;        
        decltype(line.find('='))      pos = 0;
        decltype(ConfigVector.size()) idx = 0;

        while (fr.ReadLine(line))
        {
            if (line.size() > 0)
            {
                pos = line.find('=');
                if (pos > 0 && pos < (std::numeric_limits<decltype(pos)>::max()))
                {
                    ConfigVector.push_back(str);
                    idx = ConfigVector.size() - 1;

                    ConfigVector[idx].Key = std::string(line, 0, pos);
                    ConfigVector[idx].Value = std::string(line, pos + 1);
                    ConfigVector[idx].KeyLength = pos;
                    //BWrapper::Log(BWrapper::LogPriority::Debug, "ConfigManager::Load(): File=%s, Key=[%s] Value=[%s], KeyLength=%u, pos=%u", this->FileName.c_str(), ConfigVector[idx].Key.c_str(), ConfigVector[idx].Value.c_str(), ConfigVector[idx].KeyLength, pos);
                }
            }
        } 
        fr.Close();
        return true;
    }
    else
    {
        if (BWrapper::FileSearchPriority::Assets == FSearchPriority)
            logError("ConfigManager::Load(): Asset config file not found %s", this->FileName.c_str());
    }    
    fr.Close();

    // ругаться, если это assets
    return false;
}

bool ConfigManager::Save()
{
    if (BWrapper::FileSearchPriority::Assets == FSearchPriority)
    {
        logError("ConfigManager::Save(): Asset config file save is not allowed %s", this->FileName.c_str());
        return false;
    }

    auto file = BWrapper::FileOpen(this->FileName.c_str(), FSearchPriority, BWrapper::FileOpenMode::WriteBinary);
    if (file)
    {
        for (decltype(ConfigVector.size()) i = 0; i < ConfigVector.size(); i++)
            BWrapper::FileWriteFormatted(file, "%s=%s\n", ConfigVector[i].Key.c_str(), ConfigVector[i].Value.c_str());

        BWrapper::FileClose(file);
        return true;
    }
    return false;
}

bool ConfigManager::SetIntValue(const char* Key, int Value)
{
    SetValue(Key, std::to_string(Value).c_str());
    return true;
}

bool ConfigManager::SetStrValue(const char* Key, const char* Value)
{
    SetValue(Key, Value);
    return true;
}

int  ConfigManager::GetIntValue(const char* Key, int DefaultValue)
{
    auto index = GetConfigIndex(Key);

    if (index < ConfigVector.size())
    {
        return std::stoi(ConfigVector[index].Value);
    }
    else
    {
        return DefaultValue;
    }
}
std::string  ConfigManager::GetStrValue(const char* Key, const char* DefaultValue)
{
    auto index = GetConfigIndex(Key);

    if (index < ConfigVector.size())
    {
        return ConfigVector[index].Value;
    }
    else
    {
        return DefaultValue;
    }
}

ConfigManager::ConfigManager()
{
    this->Destroy();
}

ConfigManager::~ConfigManager()
{
    this->Destroy();
}

void ConfigManager::Destroy()
{
    this->Clear();
}

void ConfigManager::Clear()
{
    ConfigVector.clear();
}

void ConfigManager::PrintConfig()
{
    logInfo("=== ConfigManager::PrintConfig() ===");
    for (decltype(ConfigVector.size()) i = 0; i < ConfigVector.size(); i++)
        logInfo("ConfigManager::PrintConfig(): File=%s, Key=[%s] Value=[%s], KeyLength=%u", this->FileName.c_str(), ConfigVector[i].Key.c_str(), ConfigVector[i].Value.c_str(), ConfigVector[i].KeyLength);
}

#endif // __AKK0RD_CONFIG_MANAGER_H__