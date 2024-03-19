#include "feats/fov.hpp"
#include "feats/move_speed.hpp"
#include "globals.hpp"
#include "menu/menu.hpp"

std::vector<void *> registeredFeatures;

#define registerFeature(name)                                                                                          \
    name::init();                                                                                                      \
    Menu::registerMenu((void *)name::menu);                                                                            \
    registeredFeatures.push_back((void *)name::tick);

int MainThread(HINSTANCE hInstDLL) {
    AllocConsole();
    freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
    std::cout << "Initializing..." << std::endl;

    while (Globals::getInstance() == nullptr) {
        std::cout << "Waiting for game instance..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    Menu::init();

    registerFeature(Feats::MoveSpeed);
    registerFeature(Feats::Fov);

    while (true) {
        if (GetAsyncKeyState(VK_END) & 1) {
            break;
        }

        for (auto &feature : registeredFeatures) {
            ((void (*)())feature)();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Menu::shutdown();

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