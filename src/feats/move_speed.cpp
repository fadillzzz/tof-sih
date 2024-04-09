#include "move_speed.hpp"
#include "../globals.hpp"

namespace Feats {
    namespace MoveSpeed {
        float defaultSpeed = 0.0f;
        double minSpeed = 1.0f;
        double maxSpeed = 10000.0f;
        Config::field<double> speed;
        Config::field<bool> enabled;

        void applySpeed(float newSpeed) {
            const auto character = Globals::getCharacter();

            if (character != nullptr && !character->IsMountCharacter()) {
                character->ClientSetMaxWalkSpeed(newSpeed);
            }
        }

        void tick() {
            if (*enabled) {
                applySpeed(*speed);
            }
        }

        void init() {
            speed = Config::get<double>("/feats/moveSpeed/speed", 0.0f);
            enabled = Config::get<bool>("/feats/moveSpeed/enabled", false);
        }

        void menu() {
            if (*speed <= 0.0f) {
                const auto character = Globals::getCharacter();

                if (character != nullptr) {
                    speed = character->PlayMaxWalkSpeed;
                    defaultSpeed = *speed;
                }
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
            ImGui::Unindent();
        }
    } // namespace MoveSpeed
} // namespace Feats
