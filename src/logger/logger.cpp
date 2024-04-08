#include "logger.hpp"

namespace Logger {
    DWORD mode;

    void init() {
        AllocConsole();
        freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleMode(output, &mode);
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(output, mode);
    }

    void shutdown() {
        fclose(stdout);
        FreeConsole();
    }

    void info(std::string message) { info(std::wstring(message.begin(), message.end())); }
    void success(std::string message) { success(std::wstring(message.begin(), message.end())); }
    void error(std::string message) { error(std::wstring(message.begin(), message.end())); }
    void warning(std::string message) { warning(std::wstring(message.begin(), message.end())); }
    void debug(std::string message) { debug(std::wstring(message.begin(), message.end())); }

    void info(std::wstring message) {
        std::cout << "\033[38;2;0;111;238m[TOFInternal:INFO] ";
        std::wcout << message;
        std::cout << "\033[0m" << std::endl;
    }

    void success(std::wstring message) {
        std::cout << "\033[38;2;23;201;100m[TOFInternal:SUCCESS] ";
        std::wcout << message;
        std::cout << "\033[0m" << std::endl;
    }

    void error(std::wstring message) {
        std::cout << "\033[38;2;243;18;96m[TOFInternal:ERROR] ";
        std::wcout << message;
        std::cout << "\033[0m" << std::endl;
    }

    void warning(std::wstring message) {
        std::cout << "\033[38;2;245;165;36m[TOFInternal:WARNING] ";
        std::wcout << message;
        std::cout << "\033[0m" << std::endl;
    }

    void debug(std::wstring message) {
        std::cout << "\033[38;2;147;83;211m[TOFInternal:DEBUG] ";
        std::wcout << message;
        std::cout << "\033[0m" << std::endl;
    }

} // namespace Logger