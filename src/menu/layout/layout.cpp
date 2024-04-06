#include "layout.hpp"
#include "../../feats/chain_logging.hpp"
#include "../../feats/fov.hpp"
#include "../../feats/inf_jump.hpp"
#include "../../feats/login.hpp"
#include "../../feats/move_speed.hpp"
#include "../../feats/no_clip.hpp"
#include "../../feats/quest.hpp"
#include "../../feats/teleport_anywhere.hpp"
#include "../../feats/teleport_nucleus.hpp"
#include "../../feats/uid_edit.hpp"

namespace Menu {
    namespace Layout {
        void render() {
            ImGui::BeginTabBar("Menu");

            if (ImGui::BeginTabItem("Player")) {
                Feats::MoveSpeed::menu();
                Feats::Fov::menu();
                Feats::InfJump::menu();
                Feats::NoClip::menu();
                Feats::UidEdit::menu();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("World")) {
                Feats::TeleportNucleus::menu();
                Feats::TeleportAnywhere::menu();
                Feats::Quest::menu();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Misc")) {
                Feats::Login::menu();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Debug")) {
                Feats::ChainLogging::menu();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    } // namespace Layout
} // namespace Menu