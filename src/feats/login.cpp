#include "../globals.hpp"

namespace Feats {
    namespace Login {
        void init() { return; }
        void tick() { return; }
        void menu() {
            if (ImGui::Button("Go back to Login")) {
                const auto instance = Globals::getInstance();

                if (instance != nullptr) {
                    instance->TravelToLoginMap();
                }
            }
        }
    } // namespace Login
} // namespace Feats