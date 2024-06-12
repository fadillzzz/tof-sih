#include "no_transparency.hpp"
#include "../globals.hpp"
#include "hotkey.hpp"

namespace Feats {
    namespace NoTransparency {
        Config::field<bool> enabled;

        void toggleTransparency(bool noTransparencyEnabled) {
            const auto localPlayer = Globals::getLocalPlayer();

            if (localPlayer != nullptr && localPlayer->PlayerController != nullptr &&
                localPlayer->PlayerController->PlayerCameraManager != nullptr) {
                const auto cameraManager =
                    (SDK::ABP_CameraManager_C *)localPlayer->PlayerController->PlayerCameraManager;
                cameraManager->EnablePlayerFade(!noTransparencyEnabled);
            }
        }

        void init() {
            enabled = Config::get<bool>(confEnabled, false);
            toggleTransparency(*enabled);
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confToggleEnabled)) {
                *enabled = !*enabled;
                toggleTransparency(*enabled);
            }
        }

        void menu() {
            if (ImGui::Checkbox("No Transparency", &enabled)) {
                toggleTransparency(*enabled);
            }
        }
    } // namespace NoTransparency
} // namespace Feats