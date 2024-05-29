#include "config.hpp"

namespace Config {
    nlohmann::json config;
    std::wstring directory = L"";
    std::wstring filePath = L"";
    std::ofstream output;
    std::chrono::time_point<std::chrono::system_clock> lastSave =
        std::chrono::system_clock::now() - std::chrono::seconds(5);
    bool saveThreadStarted = false;
    bool shuttingDown = false;

    void setDirectory(std::wstring directory) { Config::directory = directory; }

    void actualSave(bool dontWriteLastSave = false) {
        output = std::ofstream(filePath);
        output << config;
        output.close();
        if (!dontWriteLastSave) {
            lastSave = std::chrono::system_clock::now();
        }
    }

    void saveLoop() {
        while (true) {
            if (shuttingDown) {
                return;
            }

            actualSave();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    void init(HMODULE handle) {
        if (directory.empty()) {
            const uint16_t pathSize = 2048;
            wchar_t path[pathSize];
            GetModuleFileName((HMODULE)handle, (LPWSTR)path, sizeof(path));
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

        shuttingDown = false;
        saveThreadStarted = false;
        save();
    }

    void save() {
        if (saveThreadStarted) {
            return;
        }

        std::thread loop(saveLoop);
        loop.detach();

        saveThreadStarted = true;
    }

    void shutdown() { shuttingDown = true; }
} // namespace Config
