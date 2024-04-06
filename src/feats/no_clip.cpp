#include "no_clip.hpp"
#include "../globals.hpp"
#include "../logger/logger.hpp"

namespace Feats {
    namespace NoClip {
        bool enabled = false;

        void init() { return; }
        void tick() {
            if (!enabled) {
                return;
            }

            const auto character = Globals::getCharacter();

            if (character == nullptr) {
                return;
            }

            const auto controller = character->GetHottaPlayerController();

            if (controller == nullptr) {
                return;
            }

            const auto cameraManager = controller->PlayerCameraManager;

            SDK::FVector moveVector(0.0, 0.0, 0.0);

            if (GetAsyncKeyState('W') & 0x8000) {
                const auto forwardVector = cameraManager->GetActorForwardVector();
                moveVector += forwardVector;
            }

            if (GetAsyncKeyState('S') & 0x8000) {
                const auto forwardVector = cameraManager->GetActorForwardVector();
                moveVector -= forwardVector;
            }

            if (GetAsyncKeyState('A') & 0x8000) {
                const auto rightVector = cameraManager->GetActorRightVector();
                moveVector -= rightVector;
            }

            if (GetAsyncKeyState('D') & 0x8000) {
                const auto rightVector = cameraManager->GetActorRightVector();
                moveVector += rightVector;
            }

            if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                const auto upVector = cameraManager->GetActorUpVector();
                moveVector += upVector;
            }

            if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) {
                const auto upVector = cameraManager->GetActorUpVector();
                moveVector -= upVector;
            }

            moveVector.Normalize();

            const auto velocity = character->CharacterMovement->MaxWalkSpeed / 100.0f;

            const auto movement = moveVector * velocity;

            // This is required to prevent the character from continuously sliding
            // after user input has stopped.
            character->CharacterMovement->Velocity = SDK::FVector(0.0, 0.0, 0.0);

            if (movement.IsZero()) {
                return;
            }

            const auto location = (SDK::FVector *)((byte *)character->CharacterMovement->UpdatedComponent + 0x1E0);
            *location += movement;
        }

        void menu() {
            if (ImGui::Checkbox("No Clip", &enabled)) {
                const auto character = Globals::getCharacter();

                if (character == nullptr) {
                    return;
                }

                if (enabled) {
                    character->CharacterMovement->SetMovementMode(SDK::EMovementMode::MOVE_Falling, 0);
                    const auto world = Globals::getWorld();

                    // Hackish solution to avoid crashing when enabling no clip in Vera Plane
                    if (world->GetName() == "Vera_P") {
                        SDK::UKismetSystemLibrary::CollectGarbage();
                        // Waiting for garbage collection to finish. There's no good way to do this, so we chose an
                        // arbitrary amount of time and hope for the best.
                        std::this_thread::sleep_for(std::chrono::milliseconds(150));
                    }

                    character->SetActorEnableCollision(false);
                } else {
                    character->CharacterMovement->SetMovementMode(SDK::EMovementMode::MOVE_Walking, 0);
                    character->SetActorEnableCollision(true);
                }
            }
        }
    } // namespace NoClip
} // namespace Feats