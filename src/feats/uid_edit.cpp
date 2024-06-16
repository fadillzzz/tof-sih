#include "uid_edit.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "../logger/logger.hpp"
#include "hotkey.hpp"

#define USE_UID(typed, uid)                                                                                            \
    std::wstring wideCustomUid = std::wstring(uid, uid + strlen(uid));                                                 \
    typed->TextBlock_RoleUID->SetText(SDK::UKismetTextLibrary::Conv_StringToText(SDK::FString(wideCustomUid.c_str())));

#define GOD_HELP_ME(enabled, typed)                                                                                    \
    if (enabled) {                                                                                                     \
        if (originalUid.size() == 0) {                                                                                 \
            originalUid = typed->TextBlock_RoleUID->GetText().ToString();                                              \
        }                                                                                                              \
        USE_UID(typed, customUid->c_str());                                                                            \
    } else {                                                                                                           \
        USE_UID(typed, originalUid.c_str());                                                                           \
    }

namespace Feats {
    namespace UidEdit {
        Config::field<bool> enabled;
        Config::field<std::string> customUid;
        std::string originalUid = "";

        void applyChanges(SDK::UQRSLUIBase *obj, bool isSettings) {
            if (isSettings) {
                GOD_HELP_ME(*enabled, static_cast<SDK::UUI_BasicSettings_C *>(obj));
            } else {
                GOD_HELP_ME(*enabled, static_cast<SDK::UUI_TopRoleID_C *>(obj));
            }
        }

        void init() {
            enabled = Config::get<bool>(confPrefix + "/enabled", false);
            customUid = Config::get<std::string>(confPrefix + "/customUid", "TOF-SIH");

            Hooks::registerHook(
                "UI_BasicSettings.UI_BasicSettings_C.Construct",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    applyChanges((SDK::UQRSLUIBase *)pObject, true);

                    return Hooks::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);

            // Hook for copying original UID to the clipboard
            Hooks::registerHook(
                "UI_BasicSettings.UI_BasicSettings_C.BndEvt__HottaButton_CopyRoleUID_K2Node_ComponentBoundEvent_0_"
                "OnButtonClickedEvent__DelegateSignature",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    ImGui::SetClipboardText(originalUid.c_str());

                    return Hooks::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);

            // Hook for handling world change resetting the UID
            Hooks::registerHook(
                "UI_TopRoleID.UI_TopRoleID_C.Construct",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    applyChanges((SDK::UQRSLUIBase *)pObject, false);

                    return Hooks::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);
        }

        void toggle() {
            const auto topRole = Globals::getObject<SDK::UUI_TopRoleID_C *>(SDK::UUI_TopRoleID_C::StaticClass());

            if (topRole != nullptr) {
                applyChanges(topRole, false);
            }
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confToggleEnabled)) {
                enabled = !*enabled;

                toggle();
            }
        }

        void menu() {
            if (ImGui::Checkbox("Enable custom UID", &enabled)) {
                toggle();
            }

            ImGui::Indent();
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
            ImGui::InputText("Custom UID", &customUid);
            ImGui::PopItemWidth();
            ImGui::Unindent();
        }
    } // namespace UidEdit
} // namespace Feats