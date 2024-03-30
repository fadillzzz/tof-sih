#include "feats/anti_anti_cheat.hpp"
#include "feats/fov.hpp"
#include "feats/inf_jump.hpp"
#include "feats/login.hpp"
#include "feats/move_speed.hpp"
#include "feats/quest.hpp"
#include "feats/teleport_anywhere.hpp"
#include "feats/teleport_nucleus.hpp"
#include "globals.hpp"
#include "hooks.hpp"
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
    Hooks::init();

    registerFeature(Feats::AntiAntiCheat);
    registerFeature(Feats::MoveSpeed);
    registerFeature(Feats::Fov);
    registerFeature(Feats::InfJump);
    registerFeature(Feats::TeleportNucleus);
    registerFeature(Feats::Quest);
    registerFeature(Feats::Login);
    registerFeature(Feats::TeleportAnywhere);

    while (true) {
        if (GetAsyncKeyState(VK_END) & 1) {
            break;
        }

        for (auto &feature : registeredFeatures) {
            ((void (*)())feature)();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Hooks::shutdown();
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