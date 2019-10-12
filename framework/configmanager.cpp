#include <fstream>
#include "configmanager.h"

std::vector<ConfigManager::ConfigStruct>::iterator ConfigManager::GetConfigIt(const char* Key) {
    auto it = std::find_if(ConfigVector.begin(), ConfigVector.end(), [&Key](const ConfigStruct& CFG) {
        if (CFG.Key == std::string(Key))
            return true;
        return false;
    });
    return it;
};

void ConfigManager::SetValue(const char* Key, const char* Value)
{
    auto it = GetConfigIt(Key);
    if (it != ConfigVector.cend())
    {
        it->Value = std::string(Value);
    }
    else
    {
        ConfigVector.emplace_back(Key, Value);
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
    FileReader fr;
    if (fr.Open(FileName.c_str(), FSearchPriority))
    {
        std::string line;
        decltype(line.find('=')) pos = 0;
        while (fr.ReadLine(line))
        {
            if (line.size() > 0)
            {
                pos = line.find('=');
                if (pos != std::string::npos)
                {
                    ConfigVector.emplace_back(std::string(line, 0, pos).c_str(), std::string(line, pos + 1).c_str());
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

        if (s1 != std::string::npos)
            res = s1;

        if (s2 != std::string::npos && res < s2)
            res = s2;

        if (res)
        {
            std::string dir = std::string(this->FileName, 0, res);
            BWrapper::DirCreate(dir.c_str());
        }
    }

    std::ofstream ofs(this->FileName.c_str(), std::ofstream::binary | std::ofstream::out);
    for (const auto& v : ConfigVector)
        ofs << v.Key << "=" << v.Value << "\n";

    ofs.close();
    return true;
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
    auto it = GetConfigIt(Key);
    if (it != ConfigVector.end())
    {
        return std::stoi(it->Value);
    }
    else
    {
        return DefaultValue;
    }
}
std::string ConfigManager::GetStrValue(const char* Key, const char* DefaultValue)
{
    auto it = GetConfigIt(Key);
    if (it != ConfigVector.end())
    {
        return it->Value;
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