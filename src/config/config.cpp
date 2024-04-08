#include "config.hpp"
#include "../logger/logger.hpp"

namespace Config {
    void init(HINSTANCE hInstDLL) {
        const uint16_t pathSize = 2048;
        wchar_t path[pathSize];
        GetModuleFileName(hInstDLL, (LPWSTR)path, sizeof(path));

        Logger::debug(path);
    }
} // namespace Config
