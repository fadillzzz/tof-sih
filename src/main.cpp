#include "main.hpp"
#include "config.hpp"
#include "feats/anti_anti_cheat.hpp"
#include "feats/chain_logging.hpp"
#include "feats/esp.hpp"
#include "feats/fov.hpp"
#include "feats/hotkey.hpp"
#include "feats/inf_jump.hpp"
#include "feats/jump_height.hpp"
#include "feats/login.hpp"
#include "feats/move_speed.hpp"
#include "feats/no_clip.hpp"
#include "feats/quest.hpp"
#include "feats/rapid_attack.hpp"
#include "feats/teleport_anywhere.hpp"
#include "feats/teleport_box.hpp"
#include "feats/teleport_nucleus.hpp"
#include "feats/uid_edit.hpp"
#include "globals.hpp"
#include "hooks.hpp"
#include "logger/logger.hpp"
#include "menu/menu.hpp"

std::vector<void *> registeredFeatures;

#define REGISTER_FEATURE(name)                                                                                         \
    name::init();                                                                                                      \
    registeredFeatures.push_back((void *)name::tick);

extern "C" __declspec(dllexport) void preMain(const wchar_t *dir) { Config::setDirectory(dir); }

int MainThread(HINSTANCE hInstDLL) {
    Logger::init();
    Logger::info("Initializing...");

    Config::init(hInstDLL);

    while (Globals::getInstance() == nullptr) {
        Logger::info("Waiting for game instance...");
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    Logger::success("Game instance found!");

    Menu::init();
    Hooks::init();

    REGISTER_FEATURE(Feats::AntiAntiCheat);
    REGISTER_FEATURE(Feats::MoveSpeed);
    REGISTER_FEATURE(Feats::Fov);
    REGISTER_FEATURE(Feats::InfJump);
    REGISTER_FEATURE(Feats::TeleportNucleus);
    REGISTER_FEATURE(Feats::Quest);
    REGISTER_FEATURE(Feats::Login);
    REGISTER_FEATURE(Feats::TeleportAnywhere);
    REGISTER_FEATURE(Feats::ChainLogging);
    REGISTER_FEATURE(Feats::NoClip);
    REGISTER_FEATURE(Feats::UidEdit);
    REGISTER_FEATURE(Feats::Hotkey);
    REGISTER_FEATURE(Feats::TeleportBox);
    REGISTER_FEATURE(Feats::JumpHeight);
    REGISTER_FEATURE(Feats::Esp);
    REGISTER_FEATURE(Feats::RapidAttack);

    while (true) {
        if (Feats::Hotkey::hotkeyPressed(confExit)) {
            break;
        }

        for (auto &feature : registeredFeatures) {
            ((void (*)())feature)();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Feats::Esp::shutdown();
    Hooks::shutdown();
    Menu::shutdown();
    Config::shutdown();
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