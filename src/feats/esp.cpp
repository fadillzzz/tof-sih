#include "esp.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "esp/box.hpp"
#include "esp/chowchow.hpp"
#include "esp/dandelion.hpp"
#include "esp/kerosenia.hpp"
#include "esp/nucleus.hpp"
#include "esp/perspective.hpp"
#include "esp/shroom.hpp"
#include "esp/watcher.hpp"
#include "hotkey.hpp"
#include "minhook/include/MinHook.h"
#include <format>
#include <mutex>
#include <ranges>

namespace Feats {
    namespace Esp {
        Config::field<bool> enabled;
        Config::field<size_t> scanDelay;
        bool shuttingDown = false;
        std::mutex actorMutex;
        std::vector<std::shared_ptr<SDK::AActor>> scannedActors;

        typedef void (*RenderCanvas)(SDK::UGameViewportClient *, SDK::UCanvas *);
        RenderCanvas oRenderCanvas = nullptr;

        SDK::UFont *font = SDK::UObject::FindObject<SDK::UFont>("Font RobotoDistanceField.RobotoDistanceField");

        void scanActors() {
            while (!shuttingDown) {
                const auto start = std::chrono::high_resolution_clock::now();

                if (*enabled) {
                    const auto world = Globals::getWorld();

                    if (world != nullptr) {
                        const std::lock_guard<std::mutex> lock(actorMutex);
                        scannedActors.clear();
                        const auto boxes = Box::getActors(world);
                        std::ranges::move(boxes, std::back_inserter(scannedActors));
                        const auto nucleus = Nucleus::getActors(world);
                        std::ranges::move(nucleus, std::back_inserter(scannedActors));
                        const auto kerosenia = Kerosenia::getActors(world);
                        std::ranges::move(kerosenia, std::back_inserter(scannedActors));
                        const auto perspective = Perspective::getActors(world);
                        std::ranges::move(perspective, std::back_inserter(scannedActors));
                        const auto watcher = Watcher::getActors(world);
                        std::ranges::move(watcher, std::back_inserter(scannedActors));
                        const auto shroom = Shroom::getActors(world);
                        std::ranges::move(shroom, std::back_inserter(scannedActors));
                        const auto dandelion = Dandelion::getActors(world);
                        std::ranges::move(dandelion, std::back_inserter(scannedActors));
                        const auto chowchow = Chowchow::getActors(world);
                        std::ranges::move(chowchow, std::back_inserter(scannedActors));
                    }
                } else {
                    const std::lock_guard<std::mutex> lock(actorMutex);
                    scannedActors.clear();
                }

                const auto end = std::chrono::high_resolution_clock::now();

                if (end - start < std::chrono::milliseconds(*scanDelay)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(*scanDelay) - (end - start));
                }
            }
        }

        void hRenderCanvas(SDK::UGameViewportClient *viewport, SDK::UCanvas *canvas) {
            if (*enabled) {
                const auto character = Globals::getCharacter();

                if (character != nullptr) {
                    const auto playerController = character->GetHottaPlayerController();
                    SDK::FVector2D characterScreenLocation;

                    playerController->ProjectWorldLocationToScreen(character->K2_GetActorLocation(),
                                                                   &characterScreenLocation, false);
                    const std::lock_guard<std::mutex> lock(actorMutex);
                    for (auto &actor : scannedActors) {
                        SDK::FVector2D screenLocation;
                        if (playerController->ProjectWorldLocationToScreen(actor->K2_GetActorLocation(),
                                                                           &screenLocation, false)) {
                            canvas->K2_DrawLine(characterScreenLocation, screenLocation, 1.0f,
                                                SDK::FLinearColor(255, 0, 0, 255));

                            SDK::FString name = SDK::UKismetStringLibrary::Conv_NameToString(actor->Name);

                            canvas->K2_DrawText(font, name, screenLocation, SDK::FVector2D(1.f, 1.f),
                                                SDK::FLinearColor(255, 0, 0, 255), 1.f, SDK::FLinearColor(0, 0, 0, 255),
                                                SDK::FVector2D(0, 0), true, true, true,
                                                SDK::FLinearColor(255, 255, 255, 255));

                            const auto distance = character->GetDistanceTo(actor.get()) / 100.f;
                            // Constructing FString directly from wchar_t * sometimes causes
                            // garbage text, so we construct FName first then convert it to FString.
                            // This might be an issue with the UnrealContainers class not zero-ing memory or something.
                            const auto distanceString = SDK::UKismetStringLibrary::Conv_NameToString(
                                SDK::UKismetStringLibrary::Conv_StringToName(
                                    std::format(L"{:.2f}m", distance).c_str()));

                            SDK::FVector2D distanceLocation = screenLocation;
                            distanceLocation.Y += font->EmScale;
                            canvas->K2_DrawText(font, distanceString, distanceLocation, SDK::FVector2D(1.f, 1.f),
                                                SDK::FLinearColor(0, 0, 255, 255), 1.f, SDK::FLinearColor(0, 0, 0, 255),
                                                SDK::FVector2D(0, 0), true, true, true,
                                                SDK::FLinearColor(255, 255, 255, 255));
                        }
                    }
                }
            }

            oRenderCanvas(viewport, canvas);
        }

        void init() {
            enabled = Config::get<bool>(confEnabled, false);
            scanDelay = Config::get<size_t>(confScanDelay, 1000);

            const auto engine = Globals::getEngine();
            const auto viewport = engine->GameViewport;

            if (viewport != nullptr) {
                const auto addr = *(uintptr_t *)((uintptr_t)(viewport->VTable) + 0x318);
                MH_CreateHook((LPVOID)addr, hRenderCanvas, (LPVOID *)&oRenderCanvas);
                MH_EnableHook((LPVOID)addr);

                std::thread scanner(scanActors);
                scanner.detach();
            }
        }

        void shutdown() {
            shuttingDown = true;
            const auto engine = Globals::getEngine();
            const auto viewport = engine->GameViewport;

            if (viewport != nullptr) {
                const auto addr = *(uintptr_t *)((uintptr_t)(viewport->VTable) + 0x318);
                MH_DisableHook((LPVOID)addr);
                MH_RemoveHook((LPVOID)addr);
            }
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confToggleEnabled)) {
                *enabled = !*enabled;
            }
        }

        void menu() {
            ImGui::Checkbox("ESP", &enabled);
            ImGui::SameLine();
            ImGui::InputScalar("Scan delay (ms)", ImGuiDataType_U64, &scanDelay);
            ImGui::Text("Do not use extremely low value for the scan delay. It will cause performance issues and could "
                        "lead to the game crashing. Recommended value is 1000ms.");
        }
    } // namespace Esp
} // namespace Feats