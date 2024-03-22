#include "move_speed.hpp"
#include "../globals.hpp"

namespace Feats {
    namespace MoveSpeed {
        float defaultSpeed = 0.0f;
        float speed = 0.0f;
        bool enabled = false;

        void tick() {
            if (enabled) {
                const auto character = Globals::getCharacter();

                if (character != nullptr) {
                    character->ClientSetMaxWalkSpeed(speed);
                }
            }
        }

        void init() { return; }

        void menu() {
            if (speed <= 0.0f) {
                const auto character = Globals::getCharacter();

                if (character != nullptr) {
                    speed = character->PlayMaxWalkSpeed;
                    defaultSpeed = speed;
                }
            }

            if (ImGui::Checkbox("Enabled", &enabled)) {
                if (!enabled) {
                    const auto character = Globals::getCharacter();

                    if (character != nullptr) {
                        character->ClientSetMaxWalkSpeed(defaultSpeed);
                    }
                }
            }

            ImGui::SameLine();
            ImGui::SliderFloat("Movement speed", &speed, 1.0f, 10000.0f);
        }
    } // namespace MoveSpeed
} // namespace Feats
