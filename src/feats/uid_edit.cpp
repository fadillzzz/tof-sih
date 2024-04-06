#include "uid_edit.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "../logger/logger.hpp"

#define USE_UID(typed, uid)                                                                                            \
    std::wstring wideCustomUid = std::wstring(uid, uid + strlen(uid));                                                 \
    typed->TextBlock_RoleUID->SetText(SDK::UKismetTextLibrary::Conv_StringToText(SDK::FString(wideCustomUid.c_str())));

#define GOD_HELP_ME(enabled, typed)                                                                                    \
    if (enabled) {                                                                                                     \
        if (strlen(originalUid) == 0) {                                                                                \
            strcpy(originalUid, typed->TextBlock_RoleUID->GetText().ToString().c_str());                               \
        }                                                                                                              \
        USE_UID(typed, customUid);                                                                                     \
    } else {                                                                                                           \
        USE_UID(typed, originalUid);                                                                                   \
    }

namespace Feats {
    namespace UidEdit {
        bool enabled = false;
        char customUid[24] = "TOF-SIH";
        char originalUid[24] = "";

        void applyChanges(SDK::UQRSLUIBase *obj, bool isSettings) {
            if (isSettings) {
                GOD_HELP_ME(enabled, static_cast<SDK::UUI_BasicSettings_C *>(obj));
            } else {
                GOD_HELP_ME(enabled, static_cast<SDK::UUI_TopRoleID_C *>(obj));
            }
        }

        void init() {
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
                    ImGui::SetClipboardText(originalUid);

                    return Hooks::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);
        }

        void tick() { return; }

        void menu() {
            if (ImGui::Checkbox("Enable custom UID", &enabled)) {
                const auto topRole = Globals::getObject<SDK::UUI_TopRoleID_C *>(SDK::UUI_TopRoleID_C::StaticClass());

                if (topRole != nullptr) {
                    applyChanges(topRole, false);
                }
            }

            ImGui::Indent();
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
            ImGui::InputText("Custom UID", customUid, sizeof(customUid));
            ImGui::PopItemWidth();
            ImGui::Unindent();
        }
    } // namespace UidEdit
} // namespace Feats