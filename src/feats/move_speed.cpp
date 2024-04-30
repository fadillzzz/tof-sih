#include "move_speed.hpp"
#include "../globals.hpp"
#include "hotkey.hpp"

namespace Feats {
    namespace MoveSpeed {
        float defaultSpeed = 0.0f;
        double minSpeed = 1.0f;
        double maxSpeed = 10000.0f;
        Config::field<double> speed;
        Config::field<bool> enabled;

        void applySpeed(float newSpeed) {
            const auto character = Globals::getCharacter();

            if (character != nullptr) {
                SDK::UQRSLCharacterMovementComponent *movement = nullptr;

                movement = (SDK::UQRSLCharacterMovementComponent *)character->CharacterMovement;

                if (character->AttachmentReplication.AttachParent != nullptr) {
                    if (character->AttachmentReplication.AttachParent->IsA(SDK::AQRSLMountCharacter::StaticClass())) {
                        const auto mount = (SDK::AQRSLMountCharacter *)character->AttachmentReplication.AttachParent;
                        movement = (SDK::UQRSLCharacterMovementComponent *)mount->CharacterMovement;

                        if (mount->IsA(SDK::AMount_Water_C::StaticClass())) {
                            // For diving vehicles
                            movement->MaxSwimSpeed = newSpeed;
                            movement->MaxAcceleration = newSpeed;
                        }
                    }
                }

                movement->MaxWalkSpeed = newSpeed;

                if (character->IsDiving()) {
                    movement->OceanSwimSpeed = newSpeed;
                    // The acceleration has to be raised as well or else the actual
                    // velocity will be capped to around 3000 (default * 2 basically)
                    movement->MaxDiveAcceleration = movement->OceanSwimSpeed * 2;
                }

                if (character->bIsDriving && character->CurrentPhysVehicle != nullptr) {
                    // For racing vehicles
                    const auto vehicle = (SDK::AHottaVehicleBase_C *)character->CurrentPhysVehicle;
                    vehicle->VehicleMaxLinearSpeed = newSpeed;
                    vehicle->Gear.EndSpeed = newSpeed;
                    vehicle->Gear.UpShift = newSpeed;
                    vehicle->Gear.MaxTorque = newSpeed;
                }
            }
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confToggleEnabled)) {
                *enabled = !*enabled;

                if (!*enabled) {
                    applySpeed(defaultSpeed);
                }
            }

            if (*enabled) {
                applySpeed(*speed);
            }
        }

        void init() {
            speed = Config::get<double>(confSpeed, 0.0f);
            enabled = Config::get<bool>(confEnabled, false);
        }

        void menu() {
            if (defaultSpeed <= 0.0f) {
                const auto character = Globals::getCharacter();

                if (character != nullptr) {
                    defaultSpeed = character->CharacterMovement->MaxWalkSpeed;
                }
            }

            if (*speed <= 0.0f) {
                *speed = defaultSpeed;
            }

            if (ImGui::Checkbox("Enable Movement Speed", &enabled)) {
                if (!*enabled) {
                    applySpeed(defaultSpeed);
                }
            }

            ImGui::Indent();
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
            ImGui::SliderScalar("Movement speed", ImGuiDataType_Double, &speed, &minSpeed, &maxSpeed);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::PushID("reset_movement_speed");
            if (ImGui::Button("Reset")) {
                *speed = defaultSpeed;
            }
            ImGui::PopID();
            ImGui::Unindent();
        }
    } // namespace MoveSpeed
} // namespace Feats
