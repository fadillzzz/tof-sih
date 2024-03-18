#include "menu/menu.hpp"

int MainThread(HINSTANCE hInstDLL) {
    AllocConsole();
    freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
    std::cout << "Initializing..." << std::endl;

    Menu::init();

    while (true) {
        if (GetAsyncKeyState(VK_END) & 1) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Menu::shutdown();

    fclose(stdout);
    FreeConsole();
    FreeLibraryAndExitThread(hInstDLL, 0);

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        HANDLE thread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hInstDLL, 0, nullptr);
        if (thread) {
            CloseHandle(thread);
        }
    }

    return true;
}