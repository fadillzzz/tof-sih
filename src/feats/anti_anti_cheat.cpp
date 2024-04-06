#include "anti_anti_cheat.hpp"
#include "../hooks.hpp"
#include "../logger/logger.hpp"

namespace Feats {
    namespace AntiAntiCheat {
        void init() {
            std::string funcNames[] = {"HottaFramework.HottaPlayerCharacter.ServerAntiPluginReport",
                                       "HottaFramework.HottaPlayerCharacter.ServerRecordAbnormalJumpSectionData",
                                       "HottaFramework.HottaCharacter.ServerCheckEquipWeapon",
                                       "HottaFramework.HottaPlayerCharacter.ServerCheckQuestRealityState",
                                       "HottaFramework.HottaPlayerCharacter.ServerRecordHarvestedItem"};

            for (const auto &funcName : funcNames) {
                Hooks::registerHook(
                    funcName,
                    [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                        Logger::success("Blocking anticheat function: " + pFunction->GetFullName());
                        return Hooks::STOP_EXECUTION;
                    });
            }
        }

        void tick() { return; }
        void menu() { return; }
    } // namespace AntiAntiCheat
} // namespace Feats