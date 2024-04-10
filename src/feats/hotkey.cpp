#include "hotkey.hpp"
#include "../logger/logger.hpp"
#include "teleport_anywhere.hpp"
#include "teleport_nucleus.hpp"
#include "uid_edit.hpp"

#define ASSIGN_BINDINGS(label, key)                                                                                    \
    bindings[label] = Config::get<std::set<ImGuiKey>>(key, {});                                                        \
    pathToKeys[key] = &bindings[label];

namespace Feats {
    namespace Hotkey {
        std::string searchFilter = "";
        std::map<std::string, Config::field<std::set<ImGuiKey>>> bindings;
        std::map<std::string, std::set<ImGuiKey> *> pathToKeys;
        std::map<ImGuiKey, std::chrono::time_point<std::chrono::system_clock>> keyLastPressTime;
        std::string bindingKey = "";

        void init() {
            ASSIGN_BINDINGS("UID editor", Feats::UidEdit::confToggleEnabled);
            ASSIGN_BINDINGS("Teleport nucleus", Feats::TeleportNucleus::confActivate);
            ASSIGN_BINDINGS("Teleport anywhere", Feats::TeleportAnywhere::confActivate);
            return;
        }

        void tick() {
            // Don't update the key press time if we're currently binding a key
            // to prevent accidentally triggering a hotkey.
            if (bindingKey != "") {
                return;
            }

            for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_GamepadStart; key = (ImGuiKey)(key + 1)) {
                if (key == ImGuiKey_Escape) {
                    continue;
                }

                if (ImGui::IsKeyPressed(key)) {
                    keyLastPressTime[key] = std::chrono::system_clock::now();
                }
            }
        }

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
            auto pressed = true;

            if (!pathToKeys.contains(key)) {
                return false;
            }

            std::vector<std::chrono::time_point<std::chrono::system_clock>> keyPressTimes = {};
            for (auto &key : *pathToKeys[key]) {
                if (!keyLastPressTime.contains(key)) {
                    return false;
                }

                keyPressTimes.push_back(keyLastPressTime[key]);
            }

            const auto minTime = std::min_element(keyPressTimes.begin(), keyPressTimes.end());
            const auto maxTime = std::max_element(keyPressTimes.begin(), keyPressTimes.end());

            auto io = ImGui::GetIO();
            auto delay = std::chrono::milliseconds((int)(io.KeyRepeatDelay * 1000));

            if ((*maxTime - *minTime > delay) || (std::chrono::system_clock::now() - *maxTime > delay)) {
                pressed = false;
            }

            if (pressed) {
                for (auto &key : *pathToKeys[key]) {
                    keyLastPressTime.erase(key);
                }
            }

            return pressed;
        }

        void menu() {
            ImGui::Text("To set a hotkey, click the button and press the desired key(s) then press ESC to save.");
            ImGui::InputText("Filter", &searchFilter);

            std::string lowerSearchFilter = searchFilter;
            std::transform(searchFilter.begin(), searchFilter.end(), lowerSearchFilter.begin(), tolower);

            for (auto &[currentBindingKey, currentImGuiKeys] : bindings) {
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

                    // Setting ID manually because changes to label value in different ticks
                    // will cause unexpected behavior which leads to incorrect binding key
                    ImGui::PushID(currentBindingKey.c_str());

                    if (ImGui::Button(label.c_str())) {
                        bindingKey = currentBindingKey;
                        currentImGuiKeys->clear();
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
        }
    } // namespace Hotkey
} // namespace Feats
