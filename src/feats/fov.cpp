#include "imgui.h"

namespace Feats {
    namespace Fov {
        static float fov = 75.0f;

        void init() { return; }

        void tick() { return; }

        void menu() {

            if (ImGui::SliderFloat("FOV", &fov, 1.0f, 180.0f)) {
                const auto engine = (SDK::UHottaGameEngine *)SDK::UEngine::GetEngine();

                if (engine != nullptr) {
                    const auto localPlayers = engine->GameInstance->LocalPlayers;

                    if (localPlayers.Num() > 0) {
                        const auto localPlayer = (SDK::UQRSLLocalPlayer *)localPlayers[0];
                        const auto cameraManager =
                            (SDK::AHottaPlayerCameraManager *)localPlayer->PlayerController->PlayerCameraManager;

                        cameraManager->GMSetFOV(fov);
                    }
                }
            }
        }
    } // namespace Fov
} // namespace Feats