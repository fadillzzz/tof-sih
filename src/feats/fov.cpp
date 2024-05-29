#include "../globals.hpp"

namespace Feats {
    namespace Fov {
        Config::field<double> fov;
        double min = 1.0l;
        double max = 180.0l;

        void init() { fov = Config::get<double>("/feats/fov/fov", 75.0f); }

        void tick() {
            const auto world = Globals::getWorld();

            if (world != nullptr) {
                const auto localPlayer = Globals::getLocalPlayer();

                if (localPlayer != nullptr && localPlayer->PlayerController != nullptr &&
                    localPlayer->PlayerController->PlayerCameraManager != nullptr) {
                    const auto cameraManager = localPlayer->PlayerController->PlayerCameraManager;

                    if (cameraManager->GetName().find("BP_CameraManager_C") != std::string::npos) {
                        ((SDK::ABP_CameraManager_C *)cameraManager)->GMSetFOV(*fov);
                    }
                }
            }
        }

        void menu() {
            ImGui::SliderScalar("FOV", ImGuiDataType_Double, &fov, &min, &max);
            ImGui::SameLine();
            ImGui::PushID("reset_fov");
            if (ImGui::Button("Reset")) {
                *fov = 75.0f;
            }
            ImGui::PopID();
        }
    } // namespace Fov
} // namespace Feats