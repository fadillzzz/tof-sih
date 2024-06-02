#include "hotkey.hpp"
#include "../logger/logger.hpp"
#include "../main.hpp"
#include "esp.hpp"
#include "jump_height.hpp"
#include "move_speed.hpp"
#include "no_clip.hpp"
#include "quest.hpp"
#include "rapid_attack.hpp"
#include "teleport_anywhere.hpp"
#include "teleport_box.hpp"
#include "teleport_nucleus.hpp"
#include "uid_edit.hpp"

#define ASSIGN_BINDINGS(label, key, def)                                                                               \
    bindings[label] = Config::get<std::set<ImGuiKey>>(key, def);                                                       \
    pathToKeys[key] = &bindings[label];                                                                                \
    pathToNextActivation[key] = std::chrono::system_clock::now();

#define ASSIGN_BINDINGS_EMPTY_DEFAULT(label, key) ASSIGN_BINDINGS(label, key, {})

namespace Feats {
    namespace Hotkey {
        std::string searchFilter = "";
        std::map<std::string, Config::field<std::set<ImGuiKey>>> bindings;
        std::map<std::string, std::set<ImGuiKey> *> pathToKeys;
        std::map<std::string, std::chrono::time_point<std::chrono::system_clock>> pathToNextActivation;
        std::string bindingKey = "";

        void init() {
            ASSIGN_BINDINGS_EMPTY_DEFAULT("UID editor", Feats::UidEdit::confToggleEnabled);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Teleport nucleus", Feats::TeleportNucleus::confActivate);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Teleport anywhere", Feats::TeleportAnywhere::confActivate);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Complete main quest(s)", Feats::Quest::confActivateMain);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Complete Daily", Feats::Quest::confActivateDaily);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Complete Weekly", Feats::Quest::confActivateWeekly);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Complete crew missions", Feats::Quest::confActivateCrewMissions);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Complete mentorship quests", Feats::Quest::confActivateMentorship);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Complete All", Feats::Quest::confActivateAll);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("No clip", Feats::NoClip::confToggleEnabled);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Movement speed", Feats::MoveSpeed::confToggleEnabled);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Unload menu", confExit);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Teleport to nearby box", Feats::TeleportBox::confActivate);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Toggle jump height", Feats::JumpHeight::confToggleEnabled);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Toggle ESP", Feats::Esp::confToggleEnabled);
            ASSIGN_BINDINGS_EMPTY_DEFAULT("Rapid attack", Feats::RapidAttack::confToggleEnabled);
            ASSIGN_BINDINGS("Toggle menu", confToggle, {defaultToggleKey});
        }

        void tick() { return; }

        std::string getKeysString(std::set<ImGuiKey> keys) {
            std::vector<std::string> keyForLabels = {};
            for (auto &key : keys) {
                keyForLabels.push_back(ImGui::GetKeyName((ImGuiKey)key));
            }

            if (keyForLabels.size() > 0) {
                std::stringstream tmp;
                std::copy(keyForLabels.begin(), keyForLabels.end(), std::ostream_iterator<std::string>(tmp, "+"));
                std::string result = tmp.str();
                result = result.substr(0, result.size() - 1);
                return result;
            }

            return "";
        }

        bool hotkeyPressed(const std::string &key) {
            // Don't update the key press time if we're currently binding a key
            // to prevent accidentally triggering a hotkey.
            if (bindingKey != "") {
                return false;
            }

            if (!pathToKeys.contains(key)) {
                return false;
            }

            if (pathToKeys[key]->empty()) {
                return false;
            }

            if (std::chrono::system_clock::now() < pathToNextActivation[key]) {
                return false;
            }

            for (auto &key : *pathToKeys[key]) {
                if (!ImGui::IsKeyPressed(key)) {
                    return false;
                }
            }

            pathToNextActivation[key] = std::chrono::system_clock::now() + std::chrono::milliseconds(250);

            return true;
        }

        void menu() {
            ImGui::Text("To set a hotkey, click the button and press the desired key(s) then press ESC to save.");
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
            ImGui::InputText("Filter", &searchFilter);
            ImGui::PopItemWidth();

            std::string lowerSearchFilter = searchFilter;
            std::transform(searchFilter.begin(), searchFilter.end(), lowerSearchFilter.begin(), tolower);

            ImGui::BeginTable("Hotkey table", 2, ImGuiTableFlags_RowBg);

            for (auto &[currentBindingKey, currentImGuiKeys] : bindings) {
                ImGui::TableNextColumn();
                std::string lowerBindingKey = currentBindingKey;
                std::transform(currentBindingKey.begin(), currentBindingKey.end(), lowerBindingKey.begin(), tolower);

                if (searchFilter.empty() || lowerBindingKey.contains(searchFilter)) {
                    ImGui::Text(currentBindingKey.c_str());
                    ImGui::SameLine();

                    std::string label = "Edit";
                    if (bindingKey == currentBindingKey) {
                        label = "Press a key";

                        for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_GamepadStart;
                             key = (ImGuiKey)(key + 1)) {
                            if (key == ImGuiKey_Escape) {
                                continue;
                            }

                            if (!ImGui::IsKeyDown(key)) {
                                continue;
                            }

                            currentImGuiKeys->insert(key);
                        }
                    }

                    if (currentImGuiKeys->size() > 0) {
                        auto keysString = getKeysString(*currentImGuiKeys);
                        label = keysString;
                    }

                    if (bindingKey == currentBindingKey) {
                        auto buttonColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
                        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
                    }

                    ImGui::TableNextColumn();

                    // Setting ID manually because changes to label value in different ticks
                    // will cause unexpected behavior which leads to incorrect binding key
                    ImGui::PushID(currentBindingKey.c_str());

                    if (ImGui::Button(label.c_str())) {
                        bindingKey = currentBindingKey;
                        currentImGuiKeys->clear();
                        // Hack: Triggers an update to the underlying data in the config
                        // so that the changes are saved.
                        *currentImGuiKeys;
                    }

                    ImGui::PopID();

                    if (bindingKey == currentBindingKey) {
                        ImGui::PopStyleColor();
                    }

                    // Reset the state if the user has finished binding the key
                    if (ImGui::IsKeyReleased(ImGuiKey_Escape) && bindingKey == currentBindingKey) {
                        bindingKey = "";
                    }
                }
            }

            ImGui::EndTable();
        }
    } // namespace Hotkey
} // namespace Feats
