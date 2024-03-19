#include "../globals.hpp"

namespace Feats {
    namespace InfJump {
        bool enabled = false;

        void init() {}

        void tick() {
            const auto character = Globals::getCharacter();

            if (character != nullptr && enabled) {
                character->JumpCurrentCount = 0;
            }
        }

        void menu() { ImGui::Checkbox("Infinite Jump", &enabled); }
    } // namespace InfJump
} // namespace Feats