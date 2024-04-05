#include "feats/anti_anti_cheat.hpp"
#include "feats/chain_logging.hpp"
#include "feats/fov.hpp"
#include "feats/inf_jump.hpp"
#include "feats/login.hpp"
#include "feats/move_speed.hpp"
#include "feats/quest.hpp"
#include "feats/teleport_anywhere.hpp"
#include "feats/teleport_nucleus.hpp"
#include "globals.hpp"
#include "hooks.hpp"
#include "logger/logger.hpp"
#include "menu/menu.hpp"

std::vector<void *> registeredFeatures;

#define registerFeature(name)                                                                                          \
    name::init();                                                                                                      \
    registeredFeatures.push_back((void *)name::tick);

int MainThread(HINSTANCE hInstDLL) {
    Logger::init();

    Logger::info("Initializing...");

    while (Globals::getInstance() == nullptr) {
        Logger::info("Waiting for game instance...");
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    Logger::success("Game instance found!");

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
    registerFeature(Feats::ChainLogging);

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

    Logger::shutdown();
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