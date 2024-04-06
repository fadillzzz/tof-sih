#include "uid_edit.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "../logger/logger.hpp"

#define GOD_HELP_ME(enabled, typed)                                                                                    \
    if (enabled) {                                                                                                     \
        if (strlen(originalUid) == 0) {                                                                                \
            strcpy(originalUid, typed->TextBlock_RoleUID->GetText().ToString().c_str());                               \
        }                                                                                                              \
                                                                                                                       \
        std::wstring wideCustomUid = std::wstring(customUid, customUid + strlen(customUid));                           \
        typed->TextBlock_RoleUID->SetText(                                                                             \
            SDK::UKismetTextLibrary::Conv_StringToText(SDK::FString(wideCustomUid.c_str())));                          \
    } else {                                                                                                           \
        std::wstring wideCustomUid = std::wstring(originalUid, originalUid + strlen(originalUid));                     \
        typed->TextBlock_RoleUID->SetText(                                                                             \
            SDK::UKismetTextLibrary::Conv_StringToText(SDK::FString(wideCustomUid.c_str())));                          \
    }

namespace Feats {
    namespace UidEdit {
        bool enabled = false;
        char customUid[64] = "TOF-SIH";
        char originalUid[64] = "";

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
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> uint8_t {
                    applyChanges((SDK::UQRSLUIBase *)pObject, true);

                    return Hooks::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);

            // Hook for copying original UID to the clipboard
            Hooks::registerHook(
                "UI_BasicSettings.UI_BasicSettings_C.BndEvt__HottaButton_CopyRoleUID_K2Node_ComponentBoundEvent_0_"
                "OnButtonClickedEvent__DelegateSignature",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> uint8_t {
                    auto originalUidHandle = GlobalAlloc(GMEM_MOVEABLE, strlen(originalUid) + 1);
                    memcpy(GlobalLock(originalUidHandle), originalUid, strlen(originalUid) + 1);
                    OpenClipboard(nullptr);
                    EmptyClipboard();
                    SetClipboardData(CF_TEXT, originalUidHandle);
                    CloseClipboard();

                    return Hooks::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);
        }

        void tick() { return; }

        void menu() {
            ImGui::InputText("Custom UID", customUid, sizeof(customUid));

            ImGui::SameLine();

            if (ImGui::Checkbox("Enable custom UID", &enabled)) {
                const auto topRole = Globals::getObject<SDK::UUI_TopRoleID_C *>(SDK::UUI_TopRoleID_C::StaticClass());

                if (topRole != nullptr) {
                    applyChanges(topRole, false);
                }
            }
        }
    } // namespace UidEdit
} // namespace Feats