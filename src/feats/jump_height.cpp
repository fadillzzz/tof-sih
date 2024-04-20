#include "jump_height.hpp"
#include "../globals.hpp"
#include "hotkey.hpp"

namespace Feats {
    namespace JumpHeight {
        float defaultVelocity = 0.0f;
        double minVelocity = 1.0f;
        double maxVelocity = 10000.0f;
        Config::field<double> height;
        Config::field<bool> enabled;

        void applySpeed(float newSpeed) {
            const auto character = Globals::getCharacter();

            if (character != nullptr) {
                character->PlayJumpZVelocity = newSpeed;
                character->SecondJumpZVelocity = newSpeed;
            }
        }

        void init() {
            height = Config::get<double>(confHeight, 0.0f);
            enabled = Config::get<bool>(confEnabled, false);
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confToggleEnabled)) {
                *enabled = !*enabled;

                if (!*enabled) {
                    applySpeed(defaultVelocity);
                }
            }

            if (*enabled) {
                applySpeed(*height);
            }
        }

        void menu() {
            if (defaultVelocity <= 0.0f) {
                const auto character = Globals::getCharacter();

                if (character != nullptr) {
                    defaultVelocity = character->CharacterMovement->JumpZVelocity;
                }
            }

            if (*height <= 0.0f) {
                *height = defaultVelocity;
            }

            if (ImGui::Checkbox("Enable Jump Height", &enabled)) {
                if (!*enabled) {
                    applySpeed(defaultVelocity);
                }
            }

            ImGui::Indent();
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
            ImGui::SliderScalar("Jump height", ImGuiDataType_Double, &height, &minVelocity, &maxVelocity);
            ImGui::PopItemWidth();
            ImGui::Unindent();
        }
    } // namespace JumpHeight
} // namespace Feats
