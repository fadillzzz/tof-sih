#include "anti_anti_cheat.hpp"
#include "../hooks.hpp"
#include "../logger/logger.hpp"
#include "../memory_manager.hpp"

namespace Feats {
    namespace AntiAntiCheat {
        void init() {
            std::string funcNames[] = {"HottaFramework.HottaPlayerCharacter.ServerAntiPluginReport",
                                       "HottaFramework.HottaPlayerCharacter.ServerRecordAbnormalJumpSectionData",
                                       "HottaFramework.HottaCharacter.ServerCheckEquipWeapon",
                                       "HottaFramework.HottaPlayerCharacter.ServerCheckQuestRealityState",
                                       "HottaFramework.HottaPlayerCharacter.ClientOnTssSdkCheckMsgResult",
                                       "HottaFramework.HottaPlayerCharacter.Server_TssSdkCheckMsg",
                                       "HottaFramework.HottaPlayerCharacter.ServerReport",
                                       "HottaFramework.HottaPlayerCharacter.ServerReportClientInfo",
                                       "QRSL.QRSLPlayerController.ServerUploadBrokenPackages",
                                       "QRSL.QRSLPlayerController.ServerUploadIntactPackages",
                                       "HottaFramework.HottaPlayerController.ServerRequestAntiSpamHttpCheck",
                                       "HottaFramework.HottaPlayerController.ClientResponseAntiSpamHttpCheck",
                                       "Engine.PlayerController.ServerCheckClientPossession",
                                       "Engine.PlayerController.ServerCheckClientPossessionReliable",
                                       "QRSL.QRSLPlayerController.ClientNotifyBrokenPackages",
                                       "QRSL.QRSLPlayerController.IsMapPackageIntact",
                                       "QRSL.QRSLPlayerController.ServerExecuteConsoleCommand",
                                       "HottaFramework.HottaCharacter.ServerStartPlayBufferEffect",
                                       "HottaFramework.HottaPlayerCharacter.ClientResyncImportantProperty",
                                       "HottaFramework.HottaPlayerCharacter.CheckResprint",        
                                       "HottaFramework.HottaPlayerCharacter.ServerRecordHarvestedItem"};

            for (const auto &funcName : funcNames) {
                Hooks::registerHook(
                    funcName,
                    [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                        Logger::success("Blocking anticheat function: " + pFunction->GetFullName());
                        return Hooks::STOP_EXECUTION;
                    });
            }

            // AbnormalJumpSectionData trigger
            MemoryManager memory;
            const auto abnormalJumpSection =
                memory.PatternScan("40 55 53 56 57 41 55 41 56 41 57 48 8d 6c 24 e9 48 81 ec f0", 1);

            if (abnormalJumpSection.empty()) {
                Logger::error("Failed to find pattern for abnormal jump section");
                return;
            }

            DWORD oldProtect;
            const auto abnormalJumpSectionTriggerAddr = (uint8_t *)abnormalJumpSection[0];
            VirtualProtect(abnormalJumpSectionTriggerAddr, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
            *abnormalJumpSectionTriggerAddr = 0xC3;
        }

        void tick() { return; }
        void menu() { return; }
    } // namespace AntiAntiCheat
} // namespace Feats
