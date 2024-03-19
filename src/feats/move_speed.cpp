#include "move_speed.hpp"
#include "../globals.hpp"
#include "imgui.h"

namespace Feats {
    namespace MoveSpeed {
        static float speed = 0.0f;
        static bool enabled = false;

        SDK::AQRSLPlayerCharacter *getCharacter() {
            const auto localPlayer = Globals::getLocalPlayer();

            if (localPlayer == nullptr) {
                return nullptr;
            }

            if (localPlayer->PlayerController == nullptr) {
                return nullptr;
            }

            const auto character = (SDK::AQRSLPlayerCharacter *)localPlayer->PlayerController->Character;

            return character;
        }

        void tick() {
            if (enabled) {
                const auto character = getCharacter();

                if (character != nullptr) {
                    character->ClientSetMaxWalkSpeed(speed);
                }
            }
        }

        void init() { return; }

        void menu() {
            if (speed <= 0.0f) {
                const auto character = getCharacter();

                if (character != nullptr) {
                    speed = character->PlayMaxWalkSpeed;
                }
            }

            ImGui::Checkbox("Enabled", &enabled);
            ImGui::SameLine();
            ImGui::SliderFloat("Movement speed", &speed, 1.0f, 10000.0f);
        }
    } // namespace MoveSpeed
} // namespace Feats
