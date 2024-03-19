#include "../globals.hpp"
#include "imgui.h"

namespace Feats {
    namespace Fov {
        static float fov = 75.0f;

        void init() { return; }

        void tick() {
            const auto world = Globals::getWorld();

            if (world != nullptr) {
                const auto localPlayer = Globals::getLocalPlayer();

                if (localPlayer != nullptr && localPlayer->PlayerController != nullptr &&
                    localPlayer->PlayerController->PlayerCameraManager != nullptr) {
                    const auto cameraManager = localPlayer->PlayerController->PlayerCameraManager;

                    if (cameraManager->GetName().find("BP_CameraManager_C") != std::string::npos) {
                        ((SDK::ABP_CameraManager_C *)cameraManager)->GMSetFOV(fov);
                    }
                }
            }
        }

        void menu() { ImGui::SliderFloat("FOV", &fov, 1.0f, 180.0f); }
    } // namespace Fov
} // namespace Feats