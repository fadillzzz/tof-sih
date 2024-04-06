#include "move_speed.hpp"
#include "../globals.hpp"

namespace Feats {
    namespace MoveSpeed {
        float defaultSpeed = 0.0f;
        float speed = 0.0f;
        bool enabled = false;

        void applySpeed(float newSpeed) {
            const auto character = Globals::getCharacter();

            if (character != nullptr && !character->IsMountCharacter()) {
                character->ClientSetMaxWalkSpeed(newSpeed);
            }
        }

        void tick() {
            if (enabled) {
                applySpeed(speed);
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

            if (ImGui::Checkbox("Enable Movement Speed", &enabled)) {
                if (!enabled) {
                    applySpeed(defaultSpeed);
                }
            }

            ImGui::Indent();
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
            ImGui::SliderFloat("Movement speed", &speed, 1.0f, 10000.0f);
            ImGui::PopItemWidth();
            ImGui::Unindent();
        }
    } // namespace MoveSpeed
} // namespace Feats
