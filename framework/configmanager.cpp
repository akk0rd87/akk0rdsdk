#include "configmanager.h"

unsigned ConfigManager::GetConfigIndex(const char* Key)
{
    unsigned i = 0;

    for(const auto& v : ConfigVector)
    {
        if (Key == v.Key)
            return i;
        ++i;
    };
    
    return (std::numeric_limits<unsigned>::max)();
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

    {// создаем директорию для файла, если она еще не существует
        decltype(FileName.rfind("/")) s1 = this->FileName.rfind("/");
        decltype(FileName.rfind("/")) s2 = this->FileName.rfind("\\");
        
        decltype(s1) res = 0;
        
        if(s1 != std::string::npos)
            res = s1;
        
        if (s2 != std::string::npos && res < s2)
            res = s2;
        
        if(res)
        {
            std::string dir = std::string(this->FileName, 0, res);
            BWrapper::DirCreate(dir.c_str());
        }
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
        logInfo("ConfigManager::PrintConfig(): File=%s, Key=[%s] Value=[%s]", this->FileName.c_str(), ConfigVector[i].Key.c_str(), ConfigVector[i].Value.c_str());
}
