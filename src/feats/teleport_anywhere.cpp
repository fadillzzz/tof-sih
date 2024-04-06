#include "teleport_anywhere.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"

namespace Feats {
    namespace TeleportAnywhere {
        float zAxis = 0.0f;

        void init() {
            Hooks::registerHook("UI_OverviewMapContainer_WarFog_BP.UI_OverviewMapContainer_WarFog_BP_C.BP_OnMapClicked",
                                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                                    // If the T key is held down
                                    if (GetAsyncKeyState(0x54) < 0) {
                                        const auto typedParams = ((
                                            SDK::Params::UI_OverviewMapContainer_WarFog_BP_C_BP_OnMapClicked *)pParams);

                                        const auto character = Globals::getCharacter();
                                        auto rotation = character->K2_GetActorRotation();
                                        typedParams->InWorldLocation.Z = zAxis;

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
            ImGui::InputFloat("Teleport Anywhere Z axis (height)", &zAxis);
            ImGui::PopItemWidth();
            ImGui::Text("Press T while clicking on the map to teleport to that location.");
        }
    } // namespace TeleportAnywhere
} // namespace Feats