#include "chain_logging.hpp"
#include "../hooks.hpp"
#include "../logger/chain/chain.hpp"

namespace Feats {
    namespace ChainLogging {
        bool enabled = false;
        int callStackSize = 3;
        bool showObjFullName = false;

        constexpr ImVec4 red = ImVec4(0xf3 / 255.0, 0x12 / 255.0, 0x60 / 255.0, 1.0f);
        constexpr ImVec4 green = ImVec4(0x17 / 255.0, 0xc9 / 255.0, 0x64 / 255.0, 1.0f);
        constexpr ImVec4 blue = ImVec4(0.0f, 0x6f / 255.0, 0xee / 255.0, 1.0f);

        void init() {
            Hooks::registerHook(
                "*",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> uint8_t {
                    const auto functionName = pFunction->GetFullName().substr(9);
                    Logger::Chain::startCallLog(functionName, {{"objFullName", pObject->GetFullName()}});
                    return Hooks::ExecutionFlag::CONTINUE_EXECUTION;
                },
                Hooks::Type::PRE);

            Hooks::registerHook(
                "*",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> uint8_t {
                    const auto functionName = pFunction->GetFullName().substr(9);
                    Logger::Chain::endCallLog(functionName);
                    return Hooks::ExecutionFlag::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);
        }

        void tick() { return; }

        void renderStack(Logger::Chain::Call &call, bool isRoot = false, std::string index = "") {
            const auto childCount = std::get<uint64_t>(call.attributes["childCount"]);
            std::string header = call.funcName + " (" + std::to_string(childCount + 1) + " calls)";

            bool shouldRender = true;

            if (childCount > 0) {
                if (isRoot) {
                    shouldRender = ImGui::CollapsingHeader((index + header).c_str());
                } else {
                    shouldRender = ImGui::TreeNode((index + header).c_str());
                }
            }

            const auto isTreeNode = childCount > 0 && !isRoot;

            if (shouldRender) {
                if (!isTreeNode) {
                    ImGui::Indent();
                }

                auto entry = call.funcName;

                if (showObjFullName) {
                    const auto index = std::get<std::string>(call.attributes["objFullName"]).find_first_of(" ");
                    const auto type = std::get<std::string>(call.attributes["objFullName"]).substr(0, index);

                    ImGui::PushStyleColor(ImGuiCol_Text, red);
                    ImGui::Text(type.c_str());
                    ImGui::PopStyleColor();
                    ImGui::SameLine();

                    if (index != std::string::npos) {
                        const auto path = std::get<std::string>(call.attributes["objFullName"]).substr(index);

                        ImGui::PushStyleColor(ImGuiCol_Text, green);
                        ImGui::Text(path.c_str());
                        ImGui::PopStyleColor();
                        ImGui::SameLine();
                    }
                }

                ImGui::PushStyleColor(ImGuiCol_Text, blue);
                ImGui::Text(call.funcName.c_str());
                ImGui::PopStyleColor();

                for (auto i = 1; auto &child : call.children) {
                    renderStack(child, false, index + std::to_string(i) + ".");
                    i++;
                }

                if (!isTreeNode) {
                    ImGui::Unindent();
                }

                if (isTreeNode) {
                    ImGui::TreePop();
                }
            }
        }

        void menu() {
            if (ImGui::Checkbox("Chain Logging", &enabled)) {
                if (enabled) {
                    Logger::Chain::enable();
                } else {
                    Logger::Chain::disable();
                }
            }

            ImGui::SameLine();

            ImGui::PushItemWidth(100.0);
            if (ImGui::InputInt("Min Call Stack Size", &callStackSize)) {
                Logger::Chain::setMinCallStackSize((uint16_t)callStackSize);
            }
            ImGui::PopItemWidth();

            ImGui::SameLine();

            ImGui::Checkbox("Show Object Full Name", &showObjFullName);

            ImGui::SameLine();

            if (ImGui::Button("Clear Logs")) {
                Logger::Chain::clearLogs();
            }

            auto logs = Logger::Chain::getLogs();

            if (logs.empty()) {
                return;
            }

            for (auto i = 1; auto &log : logs) {
                renderStack(log, true, std::to_string(i) + ".");
                i++;
            }
        }
    } // namespace ChainLogging
} // namespace Feats
