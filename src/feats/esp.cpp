#include "esp.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "esp/box.hpp"
#include "esp/chowchow.hpp"
#include "esp/dandelion.hpp"
#include "esp/fish_baiter.hpp"
#include "esp/kerosenia.hpp"
#include "esp/nucleus.hpp"
#include "esp/particle_fish.hpp"
#include "esp/perspective.hpp"
#include "esp/shroom.hpp"
#include "esp/sponge.hpp"
#include "esp/watcher.hpp"
#include "hotkey.hpp"
#include "minhook/include/MinHook.h"
#include <iterator>
#include <shared_mutex>

namespace Feats {
    namespace Esp {
        Config::field<bool> enabled;
        Config::field<size_t> scanDelay;

        std::atomic<bool> shuttingDown{false};
        std::atomic<bool> scannerRunning{false};

        std::shared_mutex actorMutex;

        struct ActorData {
            std::vector<SDK::FVector> locations;
            std::vector<SDK::FString> names;
        };

        ActorData currentActorData;
        ActorData scanningActorData;

        std::thread scannerThread;

        typedef void (*RenderCanvas)(SDK::UGameViewportClient *, SDK::UCanvas *);
        RenderCanvas oRenderCanvas = nullptr;

        SDK::UFont *font = nullptr;

        bool hookInstalled = false;

        void scanActors() {
            scannerRunning = true;

            while (!shuttingDown) {
                auto start = std::chrono::high_resolution_clock::now();

                if (*enabled) {
                    SDK::UWorld *world = Globals::getWorld();
                    if (!world) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(*scanDelay));
                        continue;
                    }

                    scanningActorData.locations.clear();
                    scanningActorData.names.clear();

                    std::vector<SDK::TWeakObjectPtr<SDK::AActor>> scannedActors;

                    auto collectActors = [&scannedActors](const auto &actors) {
                        if (!actors.empty()) {
                            scannedActors.insert(scannedActors.end(), std::make_move_iterator(actors.begin()),
                                                 std::make_move_iterator(actors.end()));
                        }
                    };

                    try {
                        collectActors(Box::getActors(world));
                        collectActors(Nucleus::getActors(world));
                        collectActors(Kerosenia::getActors(world));
                        collectActors(Perspective::getActors(world));
                        collectActors(Watcher::getActors(world));
                        collectActors(Shroom::getActors(world));
                        collectActors(Dandelion::getActors(world));
                        collectActors(Chowchow::getActors(world));
                        collectActors(Sponge::getActors(world));
                        collectActors(ParticleFish::getActors(world));
                        collectActors(FishBaiter::getActors(world));
                    } catch (const std::exception &e) {
                        Logger::warning(std::format("Error occured while scanning for actors for ESP: {}", e.what()));
                        continue;
                    }

                    for (auto &actor : scannedActors) {
                        if (actor) {
                            try {
                                SDK::FVector location = actor->K2_GetActorLocation();
                                scanningActorData.locations.push_back(location);
                                scanningActorData.names.push_back(
                                    SDK::UKismetStringLibrary::Conv_NameToString(actor->Name));
                            } catch (const std::exception &e) {
                                Logger::warning(std::format(
                                    "Error occured while getting actor location or name for ESP: {}", e.what()));
                                continue;
                            }
                        }
                    }

                    {
                        std::unique_lock<std::shared_mutex> lock(actorMutex);
                        currentActorData.locations.swap(scanningActorData.locations);
                        currentActorData.names.swap(scanningActorData.names);
                    }
                }

                auto end = std::chrono::high_resolution_clock::now();
                auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                size_t delayMs = *scanDelay;
                if (elapsedMs < delayMs) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs - elapsedMs));
                }
            }

            scannerRunning = false;
        }

        void hRenderCanvas(SDK::UGameViewportClient *viewport, SDK::UCanvas *canvas) {
            // call original func first
            if (oRenderCanvas) {
                oRenderCanvas(viewport, canvas);
            }

            // render
            if (*enabled && canvas) {
                SDK::APlayerController *playerController = Globals::getPlayerController();
                if (!playerController) {
                    return;
                }

                SDK::AQRSLPlayerCharacter *character = Globals::getCharacter();
                if (!character) {
                    return;
                }

                if (!font) {
                    font = SDK::UObject::FindObject<SDK::UFont>("Font RobotoDistanceField.RobotoDistanceField");
                    if (!font) {
                        // Shouldn't really happen, but just in case
                        return;
                    }
                }

                SDK::FVector characterLocation = character->K2_GetActorLocation();
                SDK::FVector2D characterScreenLocation;

                if (!playerController->ProjectWorldLocationToScreen(characterLocation, &characterScreenLocation,
                                                                    false)) {
                    return;
                }

                std::vector<SDK::FVector> locations;
                std::vector<SDK::FString> names;

                {
                    std::shared_lock<std::shared_mutex> lock(actorMutex);
                    locations = currentActorData.locations;
                    names = currentActorData.names;
                }

                if (locations.size() != names.size()) {
                    return;
                }

                for (size_t i = 0; i < locations.size(); i++) {
                    SDK::FVector2D screenLocation;
                    if (!playerController->ProjectWorldLocationToScreen(locations[i], &screenLocation, false))
                        continue;

                    canvas->K2_DrawLine(characterScreenLocation, screenLocation, 1.0f,
                                        SDK::FLinearColor(1.f, 0.f, 0.f, 1.f));

                    canvas->K2_DrawText(font, names[i], screenLocation, SDK::FVector2D(1.f, 1.f),
                                        SDK::FLinearColor(1.f, 0.f, 0.f, 1.f), 1.0f,
                                        SDK::FLinearColor(0.f, 0.f, 0.f, 1.f), SDK::FVector2D(0.f, 0.f), true, true,
                                        true, SDK::FLinearColor(1.f, 1.f, 1.f, 1.f));

                    float distance = characterLocation.GetDistanceToInMeters(locations[i]);
                    wchar_t distanceBuffer[64];
                    swprintf(distanceBuffer, 64, L"%.2fm", distance);
                    SDK::FString distanceString = SDK::UKismetStringLibrary::Conv_NameToString(
                        SDK::UKismetStringLibrary::Conv_StringToName(distanceBuffer));

                    SDK::FVector2D distanceTextLocation = screenLocation;
                    distanceTextLocation.Y += font->EmScale / 2.0f;
                    canvas->K2_DrawText(font, distanceString, distanceTextLocation, SDK::FVector2D(1.f, 1.f),
                                        SDK::FLinearColor(0.f, 0.f, 1.f, 1.f), 1.0f,
                                        SDK::FLinearColor(0.f, 0.f, 0.f, 1.f), SDK::FVector2D(0.f, 0.f), true, true,
                                        true, SDK::FLinearColor(1.f, 1.f, 1.f, 1.f));
                }
            }
        }

        bool installHook() {
            if (hookInstalled) {
                return true;
            }

            SDK::UHottaGameEngine *engine = Globals::getEngine();
            if (!engine) {
                return false;
            }

            SDK::UGameViewportClient *viewport = engine->GameViewport;
            if (!viewport || !viewport->VTable) {
                return false;
            }

            uintptr_t addr = *(uintptr_t *)((uintptr_t)(viewport->VTable) + 0x318);
            if (!addr) {
                return false;
            }

            if (MH_CreateHook((LPVOID)addr, hRenderCanvas, (LPVOID *)&oRenderCanvas) != MH_OK) {
                return false;
            }

            if (MH_EnableHook((LPVOID)addr) != MH_OK) {
                MH_RemoveHook((LPVOID)addr);
                return false;
            }

            hookInstalled = true;
            return true;
        }

        bool removeHook() {
            if (!hookInstalled) {
                return true;
            }

            SDK::UHottaGameEngine *engine = Globals::getEngine();
            if (!engine) {
                return false;
            }

            SDK::UGameViewportClient *viewport = engine->GameViewport;
            if (!viewport || !viewport->VTable) {
                return false;
            }

            uintptr_t addr = *(uintptr_t *)((uintptr_t)(viewport->VTable) + 0x318);
            if (!addr) {
                return false;
            }

            bool success = true;
            if (MH_DisableHook((LPVOID)addr) != MH_OK) {
                success = false;
            }

            if (MH_RemoveHook((LPVOID)addr) != MH_OK) {
                success = false;
            }

            hookInstalled = false;
            return success;
        }

        void init() {
            enabled = Config::get<bool>(confEnabled, false);
            scanDelay = Config::get<size_t>(confScanDelay, 1000);

            font = SDK::UObject::FindObject<SDK::UFont>("Font RobotoDistanceField.RobotoDistanceField");

            if (installHook()) {
                if (!scannerRunning) {
                    shuttingDown = false;
                    scannerThread = std::thread(scanActors);
                }
            }
        }

        void shutdown() {
            shuttingDown = true;

            if (scannerThread.joinable()) {
                for (int i = 0; i < 20 && scannerRunning; i++) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                if (scannerRunning) {
                    scannerThread.detach();
                } else {
                    scannerThread.join();
                }
            }

            removeHook();
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confToggleEnabled)) {
                bool newState = !*enabled;
                enabled = newState;

                if (newState && !scannerRunning && !scannerThread.joinable()) {
                    shuttingDown = false;
                    scannerThread = std::thread(scanActors);
                }
            }
        }

        void menu() {
            if (ImGui::Checkbox("ESP", &enabled)) {
                if (*enabled && !scannerRunning && !scannerThread.joinable()) {
                    shuttingDown = false;
                    scannerThread = std::thread(scanActors);
                }
            }

            ImGui::SameLine();
            ImGui::InputScalar("Scan delay (ms)", ImGuiDataType_U64, &scanDelay);
            ImGui::Text("Recommended value is 1000ms. Extremely low values may affect game performance.");
        }
    } // namespace Esp
} // namespace Feats
