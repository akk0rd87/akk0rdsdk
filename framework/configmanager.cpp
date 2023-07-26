#include <filesystem>
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
    if (std::filesystem::exists(FileName)) {
        std::ifstream ifs(FileName, std::ios::binary | std::ios::in);
        if (ifs) {
            std::string line;
            decltype(line.find('=')) pos = 0;
            while (std::getline(ifs, line)) {
                if (!line.empty()) {
                    pos = line.find('=');
                    if (pos != std::string::npos) {
                        ConfigList.emplace(std::string(line, 0, pos), std::string(line, pos + 1));
                    }
                }
            }
        }
    }
    return true;
}

bool ConfigManager::Save()
{
    std::filesystem::create_directories(this->FileName.substr(0, this->FileName.find_last_of("\\/")));

    std::ofstream ofs(this->FileName.c_str(), std::ofstream::binary | std::ofstream::out);
    for (const auto& v : ConfigList) {
        ofs << v.first << '=' << v.second << '\n';
    }

    ofs.close();
    return true;
}

void ConfigManager::PrintConfig() {
    //logInfo("=== ConfigManager::PrintConfig() %s ===", this->FileName.c_str());
    //std::for_each(ConfigList.cbegin(), ConfigList.cend(), [](const auto& v) { logDebug("%s=%s", v.first.c_str(), v.second.c_str()); });
}