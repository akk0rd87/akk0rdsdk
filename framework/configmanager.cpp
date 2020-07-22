#include <fstream>
#include "configmanager.h"

void ConfigManager::SetValue(const char* Key, const char* Value)
{
    auto it = GetConfigIt(Key);
    if (it != ConfigList.cend()) {
        it->second = Value;
    }
    else {
        ConfigList.emplace(Key, Value);
    }
}

bool ConfigManager::Load() {
    this->Clear();
    FileReader fr;
    if (fr.Open(FileName.c_str(), FSearchPriority)) {
        std::string line;
        decltype(line.find('=')) pos = 0;
        while (fr.ReadLine(line)) {
            if (!line.empty()) {
                pos = line.find('=');
                if (pos != std::string::npos) {
                    ConfigList.emplace(std::string(line, 0, pos).c_str(), std::string(line, pos + 1).c_str());
                }
            }
        }
        fr.Close();
        return true;
    }
    else {
        if (BWrapper::FileSearchPriority::Assets == FSearchPriority) {
            logError("ConfigManager::Load(): Asset config file not found %s", this->FileName.c_str());
        }
    }
    fr.Close();

    // ругаться, если это assets
    return false;
}

bool ConfigManager::Save()
{
    if (BWrapper::FileSearchPriority::Assets == FSearchPriority) {
        logError("Asset config file save is not allowed %s", this->FileName.c_str());
        return false;
    }

    BWrapper::DirCreate(this->FileName.substr(0, this->FileName.find_last_of("\\/")).c_str());

    std::ofstream ofs(this->FileName.c_str(), std::ofstream::binary | std::ofstream::out);
    for (const auto& v : ConfigList) {
        ofs << v.first << '=' << v.second << '\n';
    }

    ofs.close();
    return true;
}

void ConfigManager::PrintConfig() {
    logInfo("=== ConfigManager::PrintConfig() ===");
    for (const auto& v : ConfigList) {
        logDebug("File=%s, Key=[%s] Value=[%s]", this->FileName.c_str(), v.first.c_str(), v.second.c_str());
    }
}