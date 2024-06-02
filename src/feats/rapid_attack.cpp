#include "rapid_attack.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "../logger/logger.hpp"
#include "../memory_manager.hpp"
#include "hotkey.hpp"

namespace Feats {
    namespace RapidAttack {
        uint8_t *rapidAttackAddress = 0;

        Config::field<bool> enabled;
        Config::field<bool> disableAnim;

        DWORD oldProtect;

        void init() {
            enabled = Config::get<bool>(confEnabled, false);
            disableAnim = Config::get<bool>(confDisableCharAnim, false);

            MemoryManager memory;
            const auto result = memory.PatternScan("0F 28 ? 74 ? F3 0F 10 4F ? F3 0F 5C 4C 24 ? 0F 2F D1", 1);

            if (result.empty()) {
                Logger::error("Failed to find pattern for rapid attack");
                return;
            }

            rapidAttackAddress = (uint8_t *)result[0];

            VirtualProtect(rapidAttackAddress + 2, 1, PAGE_EXECUTE_READWRITE, &oldProtect);

            Hooks::registerHook(
                "SendAttackTargNS.SendAttackTargNS_C.Received_NotifyBegin",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    const auto character = Globals::getCharacter();

                    if (character != nullptr) {
                        const auto skillComponent = character->SkillComponent;

                        if (skillComponent != nullptr) {
                            const auto localAnimMontageInfo = skillComponent->LocalAnimMontageInfo;
                            const auto animMontage = localAnimMontageInfo.AnimMontage;

                            if (animMontage != nullptr) {
                                if (*enabled && *disableAnim && animMontage->SlotAnimTracks.Num() > 0) {
                                    animMontage->SlotAnimTracks.Remove(0);
                                }

                                if ((!*enabled || !*disableAnim) && animMontage->SlotAnimTracks.Num() == 0) {
                                    animMontage->SlotAnimTracks.IncrementNum();
                                }
                            }
                        }
                    }

                    return Hooks::CONTINUE_EXECUTION;
                },
                Hooks::POST);
        }

        void toggle(bool isEnabled) {
            if (isEnabled) {
                // movaps xmm0, xmm0
                *(rapidAttackAddress + 2) = 0xC0;
            } else {
                // movaps xmm0, xmm2
                *(rapidAttackAddress + 2) = 0xC2;
            }
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confToggleEnabled)) {
                *enabled = !*enabled;
                toggle(*enabled);
            }
        }

        void menu() {
            if (ImGui::Checkbox("Rapid Attack (fast weapon animation)", &enabled)) {
                toggle(*enabled);
            }

            ImGui::Indent();
            ImGui::Checkbox("Disable character animation", &disableAnim);
            ImGui::Unindent();
        }
    } // namespace RapidAttack
} // namespace Feats
