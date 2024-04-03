#include "rapid_attack.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "../logger/logger.hpp"
#include "../memory_manager.hpp"

namespace Feats {
    namespace RapidAttack {
        uint8_t *rapidAttackAddress = 0;
        bool enabled = false;
        DWORD oldProtect;

        void init() {
            MemoryManager memory;
            const auto result = memory.PatternScan("0F 28 ? 74 ? F3 0F 10 4F ? F3 0F 5C 4C 24 ? 0F 2F D1", 1);

            if (result.empty()) {
                Logger::error("Failed to find pattern for rapid attack");
                return;
            }

            rapidAttackAddress = (uint8_t *)result[0];

            VirtualProtect(rapidAttackAddress + 2, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
        }

        void tick() { return; }

        void menu() {
            if (ImGui::Checkbox("Rapid Attack", &enabled)) {
                if (enabled) {
                    // movaps xmm0, xmm0
                    *(rapidAttackAddress + 2) = 0xC0;
                } else {
                    // movaps xmm0, xmm2
                    *(rapidAttackAddress + 2) = 0xC2;
                }
            }
        }
    } // namespace RapidAttack
} // namespace Feats
