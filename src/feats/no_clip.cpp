#include "no_clip.hpp"
#include "../globals.hpp"
#include "../logger/logger.hpp"

namespace Feats {
    namespace NoClip {
        bool enabled = false;
        float defaultGravityScale = 0.0;
        float velocity = 5.0;

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

            moveVector = moveVector.Normalize();

            const auto movement = moveVector * velocity;

            // This has to be set every tick, because certain movements can
            // cause the value to be reset to the default value.
            character->CharacterMovement->GravityScale = 0.0;

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
                    character->CapsuleComponent->SetCollisionEnabled(SDK::ECollisionEnabled::NoCollision);
                    character->CharacterMovement->SetMovementMode(SDK::EMovementMode::MOVE_Falling, 0);
                    defaultGravityScale = character->CharacterMovement->GravityScale;
                } else {
                    character->CharacterMovement->SetMovementMode(SDK::EMovementMode::MOVE_Walking, 0);
                    character->CapsuleComponent->SetCollisionEnabled(SDK::ECollisionEnabled::QueryAndPhysics);
                    character->CharacterMovement->GravityScale = defaultGravityScale;
                }
            }

            ImGui::SameLine();

            ImGui::SliderFloat("Velocity", &velocity, 0.0, 100.0);
        }
    } // namespace NoClip
} // namespace Feats