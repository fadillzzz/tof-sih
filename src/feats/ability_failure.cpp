#include "ability_failure.hpp"
#include "../hooks.hpp"

namespace Feats {
    namespace AbilityFailure {
        Config::field<bool> enabled;
        uint8_t locks = 0;

        void init() {
            enabled = Config::get<bool>("/feats/abilityFailure/enabled", false);

            Hooks::registerHook(
                "GameplayAbilities.AbilitySystemComponent.ClientActivateAbilityFailed",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    if (*enabled) {
                        return Hooks::ExecutionFlag::STOP_EXECUTION;
                    }

                    return Hooks::ExecutionFlag::CONTINUE_EXECUTION;
                });
        }

        void tick() { return; }

        void menu() {
            if (locks > 0) {
                ImGui::BeginDisabled();
            }

            ImGui::Checkbox("Ignore ability failure", &enabled);

            if (locks > 0) {
                ImGui::EndDisabled();
            }
        }

        void enableAndLock() {
            enabled = true;
            locks++;
        }

        void unlock() { locks--; }
    } // namespace AbilityFailure
} // namespace Feats