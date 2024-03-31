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
        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        fclose(stdout);
        FreeConsole();
    }

    void info(std::string message) {
        std::cout << "\033[38;2;0;111;238m[TOFInternal:INFO] ";
        std::cout << message;
        std::cout << "\033[0m" << std::endl;
    }

    void success(std::string message) {
        std::cout << "\033[38;2;23;201;100m[TOFInternal:SUCCESS] ";
        std::cout << message;
        std::cout << "\033[0m" << std::endl;
    }

    void error(std::string message) {
        std::cout << "\033[38;2;243;18;96m[TOFInternal:ERROR] ";
        std::cout << message;
        std::cout << "\033[0m" << std::endl;
    }

    void warning(std::string message) {
        std::cout << "\033[38;2;245;165;36m[TOFInternal:WARNING] ";
        std::cout << message;
        std::cout << "\033[0m" << std::endl;
    }

    void debug(std::string message) {
        std::cout << "\033[38;2;147;83;211m[TOFInternal:DEBUG] ";
        std::cout << message;
        std::cout << "\033[0m" << std::endl;
    }

} // namespace Logger