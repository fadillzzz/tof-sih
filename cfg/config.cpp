#include "config.hpp"

namespace Config {
    nlohmann::json config;
    std::wstring directory = L"";
    std::wstring filePath = L"";
    std::ofstream output;
    std::chrono::time_point<std::chrono::system_clock> lastSave =
        std::chrono::system_clock::now() - std::chrono::seconds(5);

    void setDirectory(std::wstring directory) { Config::directory = directory; }

    void actualSave(bool dontWriteLastSave = false) {
        output = std::ofstream(filePath);
        output << config;
        output.close();
        if (!dontWriteLastSave) {
            lastSave = std::chrono::system_clock::now();
        }
    }

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
            actualSave(true);
        } else {
            std::ifstream file(filePath);
            config = nlohmann::json::parse(file);
            file.close();
        }
    }

    void save(bool force) {
        if ((std::chrono::system_clock::now() - lastSave > std::chrono::seconds(5)) || force) {
            std::thread saveThread([]() { actualSave(); });
            saveThread.detach();
        }
    }

    void shutdown() { save(); }
} // namespace Config
