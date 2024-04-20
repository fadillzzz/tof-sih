#include "layout.hpp"
#include "../../feats/about.hpp"
#include "../../feats/chain_logging.hpp"
#include "../../feats/esp.hpp"
#include "../../feats/fov.hpp"
#include "../../feats/hotkey.hpp"
#include "../../feats/inf_jump.hpp"
#include "../../feats/jump_height.hpp"
#include "../../feats/login.hpp"
#include "../../feats/move_speed.hpp"
#include "../../feats/no_clip.hpp"
#include "../../feats/ping.hpp"
#include "../../feats/quest.hpp"
#include "../../feats/teleport_anywhere.hpp"
#include "../../feats/teleport_box.hpp"
#include "../../feats/teleport_nucleus.hpp"
#include "../../feats/uid_edit.hpp"

namespace Menu {
    namespace Layout {
        void render() {
            ImGui::BeginTabBar("Menu");

            if (ImGui::BeginTabItem("Player")) {
                Feats::MoveSpeed::menu();
                Feats::Fov::menu();
                Feats::JumpHeight::menu();
                Feats::InfJump::menu();
                Feats::NoClip::menu();
                Feats::UidEdit::menu();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("World")) {
                Feats::TeleportNucleus::menu();
                Feats::TeleportBox::menu();
                Feats::TeleportAnywhere::menu();
                Feats::Quest::menu();
                Feats::Esp::menu();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Misc")) {
                Feats::Login::menu();
                Feats::Ping::menu();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Hotkeys")) {
                Feats::Hotkey::menu();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Debug")) {
                Feats::ChainLogging::menu();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("About")) {
                Feats::About::menu();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    } // namespace Layout
} // namespace Menu