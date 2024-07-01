#include "display_poi.hpp"
#include "../globals.hpp"
#include "../sig.hpp"

namespace Feats {
    namespace DisplayPoi {
        enum ScanStatus { NOT_SCANNED = 0, SCANNING = 1, FINISHED = 2, FAILED = 3 };

        uint8_t scanStatus = NOT_SCANNED;

        void init() {}
        void tick() {}

        uintptr_t scanEx(const char *pattern) {
            MEMORY_BASIC_INFORMATION mbi;
            char *base = nullptr;
            std::vector<std::thread> threads;
            uintptr_t result = 0;
            while (VirtualQuery(base, &mbi, sizeof(mbi)) == sizeof(mbi)) {
                if (mbi.Protect == PAGE_READWRITE && mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE &&
                    mbi.RegionSize == 0x10000) {
                    threads.push_back(std::thread([mbi, &result, &pattern]() {
                        const auto found = Sig::find(mbi.BaseAddress, mbi.RegionSize, pattern);
                        if (found != nullptr) {
                            result = (uintptr_t)found;
                        }
                    }));
                }

                if (threads.size() >= std::thread::hardware_concurrency()) {
                    for (auto &thread : threads) {
                        thread.join();
                    }
                    threads.clear();
                }

                if (result != 0) {
                    break;
                }

                base += mbi.RegionSize;
            }

            if (threads.size() > 0) {
                for (auto &thread : threads) {
                    thread.join();
                }

                threads.clear();
            }

            return result;
        }

        void scan() {
            auto playerPoi = scanEx("00 00 00 00 00 01 00 01 00 00 b4 42 00 00 00 00 ?? ?? ?? ?? ?? ?? ?? ?? 01");

            if (playerPoi != 0) {
                playerPoi -= 0x38;
                auto pointer = std::format("{:016x}", _byteswap_uint64(playerPoi));
                std::string patternBuff;
                patternBuff.resize(pointer.size() + pointer.size() / 2 - 1);
                for (size_t i = 0, j = 0; i < pointer.size(); i++, j++) {
                    patternBuff[j] = pointer[i];
                    if (i % 2 == 1) {
                        patternBuff[++j] = ' ';
                    }
                }
                const auto pattern = patternBuff + " ff ff ff ff";
                const auto firstIndex = scanEx(pattern.c_str());

                if (firstIndex != 0) {
                    const auto globals = Globals::getObject<SDK::UQRSLGlobals *>(SDK::UQRSLGlobals::StaticClass());
                    const auto gameData = globals->HottaGameData;
                    const auto poiData = gameData->MiniMapPOIData;
                    SDK::TArray<SDK::FName> poiNames;
                    SDK::UDataTableFunctionLibrary::GetDataTableRowNames(poiData, &poiNames);
                    const auto totalRows = poiNames.Num();

                    SDK::FName iconClassName;
                    iconClassName.Number = 0;

                    for (size_t i = 0; i < totalRows; i++) {
                        const auto poi = *(SDK::FMiniMapPOIData **)(firstIndex + i * 0x18);
                        poi->VisibleZoomRange.LowerBound.Value = 0.0f;
                        poi->VisibleZoomRange.UpperBound.Value = 1.0f;
                        poi->ValidBigMapDistance = 0.0f;
                        poi->ValidMinimapDistance = 0.0f;
                        poi->bIsCanTransfer = true;
                        poi->bIsCanShowTips = false;
                        poi->bIsCanInteract = true;

                        if (iconClassName.Number == 0 &&
                            poi->MapIconInfoClassName.ToString() == "SkillTransferMapIconInfo") {
                            iconClassName = poi->MapIconInfoClassName;
                        }
                    }

                    for (size_t i = 0; i < totalRows; i++) {
                        const auto poi = *(SDK::FMiniMapPOIData **)(firstIndex + i * 0x18);
                        poi->MapIconInfoClassName = iconClassName;
                    }

                    scanStatus = FINISHED;
                } else {
                    scanStatus = FAILED;
                }
            } else {
                scanStatus = FAILED;
            }
        }

        void menu() {
            if (ImGui::Button("Display all detectable POI in map (slow)")) {
                if (scanStatus == NOT_SCANNED) {
                    std::thread(scan).detach();
                    scanStatus = SCANNING;
                }
            }

            ImGui::SameLine();

            if (scanStatus == SCANNING) {
                ImGui::Text("Scanning...");
            } else if (scanStatus == FINISHED) {
                ImGui::Text("Finished");
            } else if (scanStatus == FAILED) {
                ImGui::Text("Failed to scan for pattern");
            }
        }
    } // namespace DisplayPoi
} // namespace Feats