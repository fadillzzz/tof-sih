#include "layout.hpp"
#include "../../feats/ability_failure.hpp"
#include "../../feats/about.hpp"
#include "../../feats/chain_logging.hpp"
#include "../../feats/esp.hpp"
#include "../../feats/fov.hpp"
#include "../../feats/hotkey.hpp"
#include "../../feats/inf_dodge.hpp"
#include "../../feats/inf_jump.hpp"
#include "../../feats/jump_height.hpp"
#include "../../feats/login.hpp"
#include "../../feats/move_speed.hpp"
#include "../../feats/no_clip.hpp"
#include "../../feats/quest.hpp"
#include "../../feats/rapid_attack.hpp"
#include "../../feats/teleport_anywhere.hpp"
#include "../../feats/teleport_box.hpp"
#include "../../feats/teleport_nucleus.hpp"
#include "../../feats/uid_edit.hpp"

namespace Menu {
    namespace Layout {
        int selectedPlayerOption = 0;
        int selectedWorldOption = 0;

        template <size_t OptionCount>
        void tabContent(std::string label, std::array<const char *, OptionCount> sidebarOptions, int *sidebarSelection,
                        std::function<void()> content) {
            if (ImGui::BeginTabItem(label.c_str())) {
                ImGui::BeginChild((label + "Child").c_str());

                ImGui::Columns(2, (label + "Content").c_str(), false);
                ImGui::SetColumnWidth(0, 200.0f);
                ImGui::SetNextItemWidth(200.0f);
                ImGui::ListBox(("##" + label + "List").c_str(), sidebarSelection, sidebarOptions.data(),
                               sidebarOptions.size(), ImGui::GetWindowHeight() / ImGui::GetTextLineHeightWithSpacing());
                ImGui::NextColumn();
                content();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
        }

        void render() {
            ImGui::BeginTabBar("Menu");

            std::array<const char *, 3> playerOptions = {"Movement", "Visual", "Combat"};
            tabContent("Player", playerOptions, &selectedPlayerOption, []() {
                switch (selectedPlayerOption) {
                case 0:
                    Feats::MoveSpeed::menu();
                    Feats::JumpHeight::menu();
                    Feats::InfJump::menu();
                    Feats::NoClip::menu();
                    break;
                case 1:
                    Feats::Fov::menu();
                    Feats::UidEdit::menu();
                case 2:
                    Feats::RapidAttack::menu();
                    Feats::InfDodge::menu();
                    Feats::AbilityFailure::menu();
                    break;
                }
            });

            std::array<const char *, 3> worldOptions = {"Teleport", "Quest", "ESP"};
            tabContent("World", worldOptions, &selectedWorldOption, []() {
                switch (selectedWorldOption) {
                case 0:
                    Feats::TeleportNucleus::menu();
                    Feats::TeleportBox::menu();
                    Feats::TeleportAnywhere::menu();
                    break;
                case 1:
                    Feats::Quest::menu();
                    break;
                case 2:
                    Feats::Esp::menu();
                    break;
                }
            });

            if (ImGui::BeginTabItem("Misc")) {
                Feats::Login::menu();
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
