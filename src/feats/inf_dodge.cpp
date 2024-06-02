#include "inf_dodge.hpp"
#include "../memory_manager.hpp"
#include "hotkey.hpp"

namespace Feats {
    namespace InfDodge {
        Config::field<bool> enabled;
        uint8_t *infiniteDodgeAddr;

        void init() {
            enabled = Config::get<bool>("/feats/infiniteDodge/enabled", false);

            MemoryManager memory;
            const auto result = memory.PatternScan(
                "? ? 8B 0D ? ? ? ? F7 EA 0F 29 ? ? ? C1 FA ? 8B C2 C1 E8 ? 03 D0 65 48 ? ? ? ? ? ? ? 48 8B", 1);

            if (result.empty()) {
                Logger::error("Failed to find infinite dodge pattern.");
                return;
            }

            DWORD oldProtect;
            infiniteDodgeAddr = (uint8_t *)result[0];
            VirtualProtect(infiniteDodgeAddr, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
        }

        void toggleEnabled(bool isEnabled) {
            if (isEnabled) {
                infiniteDodgeAddr[0] = 0x90;
                infiniteDodgeAddr[1] = 0x90;
            } else {
                infiniteDodgeAddr[0] = 0x2B;
                infiniteDodgeAddr[1] = 0x11;
            }
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confToggleEnabled)) {
                *enabled = !*enabled;
                toggleEnabled(*enabled);
            }
        }

        void menu() {
            if (ImGui::Checkbox("Infinite Dodge", &enabled)) {
                toggleEnabled(*enabled);
            }
        }
    } // namespace InfDodge
} // namespace Feats