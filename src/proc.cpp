#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <filesystem>
#include <iostream>

#include "../injector/inject.hpp"

int main(int argc, char **argv) {
    if (argc < 2) {
        return 1;
    }

    const auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, atoi(argv[1]));
    if (!hProc) {
        return 1;
    }

    const auto path = std::filesystem::current_path().wstring() + L"\\TOFInternal.dll";
    InjectDll(hProc, path);

    return 0;
}