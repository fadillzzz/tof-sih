#include "ping.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "hotkey.hpp"

namespace Feats {
    namespace Ping {
        Config::field<bool> enabled;

        void updateUi() {
            const auto mainForm = Globals::getObject<SDK::UUI_MainForm_C *>(SDK::UUI_MainForm_C::StaticClass());

            if (mainForm != nullptr) {
                if (*enabled == true) {
                    mainForm->TextBlock_Ping->SetVisibility(SDK::ESlateVisibility::Visible);
                    mainForm->TextBlock_Ping->GetParent()->SetVisibility(SDK::ESlateVisibility::Visible);
                } else {
                    mainForm->TextBlock_Ping->SetVisibility(SDK::ESlateVisibility::SelfHitTestInvisible);
                    mainForm->TextBlock_Ping->GetParent()->SetVisibility(SDK::ESlateVisibility::Collapsed);
                }
            } else {
                Logger::error("UI_MainForm not found");
            }
        }

        void init() {
            enabled = Config::get<bool>(confEnabled, false);

            Hooks::registerHook(
                "UI_MainForm.UI_MainForm_C.OnInitialized",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    updateUi();
                    return Hooks::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confToggleEnabled)) {
                *enabled = !*enabled;
                updateUi();
            }
        }

        void menu() {
            if (ImGui::Checkbox("Show ping", &enabled)) {
                updateUi();
            }
        }
    } // namespace Ping
} // namespace Feats
