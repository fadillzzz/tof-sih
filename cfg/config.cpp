#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "config.hpp"

namespace Config {
    nlohmann::json config;
    std::wstring directory = L"";
    std::wstring filePath = L"";
    std::ofstream output;

    void setDirectory(std::wstring directory) { Config::directory = directory; }

    void init() {
        if (directory.empty()) {
            const uint16_t pathSize = 2048;
            wchar_t path[pathSize];
            GetModuleFileName(nullptr, (LPWSTR)path, sizeof(path));
            std::wstring dir = std::wstring(path);
            dir = dir.substr(0, dir.find_last_of(L"\\") + 1);
            setDirectory(dir);
        }

        filePath = directory + L"\\tof-sih.json";

        if (!std::filesystem::exists(filePath)) {
            std::ofstream file(filePath);
            file.close();
        }

        const auto size = std::filesystem::file_size(std::filesystem::path(filePath));

        if (size == 0) {
            config = nlohmann::json::object();
        } else {
            std::ifstream file(filePath);
            config = nlohmann::json::parse(file);
            file.close();
        }
    }

    void save() {
        output = std::ofstream(filePath);
        output << config;
        output.close();
    }

    void shutdown() { save(); }
} // namespace Config
