#include "teleport_anywhere.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "hotkey.hpp"

namespace Feats {
    namespace TeleportAnywhere {
        Config::field<double> zAxis;

        void init() {
            zAxis = Config::get<double>("/feats/teleportAnywhere/zAxis", 0.0f);

            Hooks::registerHook(
                "UI_OverviewMapContainer_WarFog_BP.UI_OverviewMapContainer_WarFog_BP_C.BP_OnMapClicked",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    if (Feats::Hotkey::hotkeyPressed(confActivate)) {
                        const auto typedParams =
                            ((SDK::Params::UI_OverviewMapContainer_WarFog_BP_C_BP_OnMapClicked *)pParams);

                        const auto character = Globals::getCharacter();
                        auto rotation = character->K2_GetActorRotation();
                        typedParams->InWorldLocation.Z = *zAxis;

                        const auto typedObject = (SDK::UUI_OverviewMapContainer_WarFog_BP_C *)pObject;
                        typedObject->RemoveFromParent();

                        character->TeleportWithLoading(typedParams->InWorldLocation, rotation);
                    }

                    return Hooks::CONTINUE_EXECUTION;
                });
        }

        void tick() { return; }

        void menu() {
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2f);
            ImGui::InputDouble("Teleport Anywhere Z axis (height)", &zAxis);
            ImGui::PopItemWidth();
            ImGui::Text("Teleport anywhere hotkey can be bound under the hotkey tab.");
        }
    } // namespace TeleportAnywhere
} // namespace Feats