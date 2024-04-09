#include "../globals.hpp"

namespace Feats {
    namespace InfJump {
        Config::field<bool> enabled;

        void init() { enabled = Config::get<bool>("/feats/infJump/enabled", false); }

        void tick() {
            const auto character = Globals::getCharacter();

            if (character != nullptr && *enabled) {
                character->JumpCurrentCount = 0;
            }
        }

        void menu() { ImGui::Checkbox("Infinite Jump", &enabled); }
    } // namespace InfJump
} // namespace Feats