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
#include <atomic>
#include <shared_mutex>
#include <limits>

namespace Feats {
    namespace Esp {
        Config::field<bool> enabled;
        Config::field<size_t> scanDelay;
        Config::field<bool> disableDistanceLimit;
        Config::field<float> maxDistance;
        
        Config::field<bool> drawBox;
        Config::field<bool> drawTracer;
        Config::field<bool> drawName;
        Config::field<bool> drawDistance;
        
        Config::field<float> fontSize;
        Config::field<float> outlineWidth;
        Config::field<float> tracerWidth;
        Config::field<float> transparency;
        
        Config::field<std::array<float, 4>> boxColor;
        Config::field<std::array<float, 4>> tracerColor;
        Config::field<std::array<float, 4>> textColor;
        
        Config::field<bool> showSupplyBox;
        Config::field<bool> showNucleus;
        Config::field<bool> showKerosenia;
        Config::field<bool> showPerspective;
        Config::field<bool> showWatcher;
        Config::field<bool> showShroom;
        Config::field<bool> showDandelion;
        Config::field<bool> showChowchow;
        Config::field<bool> showSponge;
        Config::field<bool> showParticleFish;
        Config::field<bool> showFishBaiter;

        std::atomic<bool> shuttingDown{false};
        std::atomic<bool> scannerRunning{false};
        
        std::shared_mutex actorMutex;

        struct ActorData {
            std::vector<SDK::FVector> locations;
            std::vector<SDK::FString> names;
            std::vector<std::string> types;
        };
        
        ActorData currentActorData;
        ActorData scanningActorData;
        
        std::thread scannerThread;

        typedef void (*RenderCanvas)(SDK::UGameViewportClient*, SDK::UCanvas*);
        RenderCanvas oRenderCanvas = nullptr;

        SDK::UFont* font = nullptr;

        bool hookInstalled = false;

        static void ForceAlwaysRelevantClasses() {
            SDK::UHottaGameEngine* engine = Globals::getEngine();
            if (!engine) return;

            SDK::UWorld* world = Globals::getWorld();
            if (!world) return;

            SDK::UNetDriver* net = world->NetDriver;
            if (!net || !net->ReplicationDriver) return;

            SDK::UReplicationGraph* baseGraph = 
                static_cast<SDK::UReplicationGraph*>(net->ReplicationDriver);
            if (!baseGraph) return;

            SDK::UQRSLReplicationGraph* graph = 
                static_cast<SDK::UQRSLReplicationGraph*>(baseGraph);
            if (!graph) return;

            static SDK::UClass* classesToForce[] = {
                SDK::AQRSLTreasureBoxActor::StaticClass(),
                SDK::ABP_MiniGame_FlyFlower_001_C::StaticClass(),
                SDK::ABP_MiniGame_ThrowFlower_002_C::StaticClass(),
                SDK::ABP_Minigame_PerspectivePuzzle_Base_C::StaticClass(),
                SDK::ABP_Minigame_PerspectivePuzzle_Sea_Base_C::StaticClass(),
                SDK::ABP_EternalWatcher_C::StaticClass(),
                SDK::ABP_SeaWatcher_C::StaticClass(),
                SDK::ABP_Minigame_PerspectivePuzzle_Base_C::StaticClass(),
                SDK::ABP_Minigame_PerspectivePuzzle_Sea_Base_C::StaticClass(),
                SDK::ABP_Fire_MiniGame_BPBase_C::StaticClass(),
                SDK::ABP_MiniGame_FireLink_Vera_C::StaticClass(),
                SDK::ABP_FishBall_Base_C::StaticClass(),
                SDK::ABP_ParticleFish_Base_C::StaticClass(),
                SDK::ABP_SeaSponge_Base_C::StaticClass(),
                SDK::ABP_LitMushroom_Manager_C::StaticClass(),
                SDK::ABP_Manager_LumenMushroom_C::StaticClass(),
                SDK::AVeraCity_Gem_BP_C::StaticClass(),
                SDK::ABP_Harvest_Gem_Base_C::StaticClass(),
                SDK::ABP_SeaSponge_Base_C::StaticClass()
            };

            for (SDK::UClass* cls : classesToForce) {
                if (cls) {
                    bool alreadyExists = false;

                    for (int i = 0; i < graph->AlwaysRelevantClasses.Num(); i++) {
                        if (graph->AlwaysRelevantClasses[i] == cls) {
                            alreadyExists = true;
                            break;
                        }
                    }

                    if (!alreadyExists) {
                        graph->AlwaysRelevantClasses.Add(cls);
                    }
                }
            }
        }


        void HelpMarker(const char* desc) {
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(desc);
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        }

        bool shouldShowActorType(const std::string& type) {
            if (type == "Box") return *showSupplyBox;
            if (type == "Nucleus") return *showNucleus;
            if (type == "Kerosenia") return *showKerosenia;
            if (type == "Perspective") return *showPerspective;
            if (type == "Watcher") return *showWatcher;
            if (type == "Shroom") return *showShroom;
            if (type == "Dandelion") return *showDandelion;
            if (type == "Chowchow") return *showChowchow;
            if (type == "Sponge") return *showSponge;
            if (type == "ParticleFish") return *showParticleFish;
            if (type == "FishBaiter") return *showFishBaiter;
            return true;
        }
        
        void scanActors() {
            scannerRunning = true;
            
            while (!shuttingDown) {
                auto start = std::chrono::high_resolution_clock::now();

                if (*enabled) {
                    SDK::UWorld* world = Globals::getWorld();
                    if (!world) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(*scanDelay));
                        continue;
                    }
                    
                    scanningActorData.locations.clear();
                    scanningActorData.names.clear();
                    scanningActorData.types.clear();
                    
                    struct ActorTypeInfo {
                        std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actors;
                        std::string type;
                    };

                    std::vector<ActorTypeInfo> actorTypes;

                    if (*showSupplyBox) actorTypes.push_back({Box::getActors(world), "Box"});
                    if (*showNucleus) actorTypes.push_back({Nucleus::getActors(world), "Nucleus"});
                    if (*showKerosenia) actorTypes.push_back({Kerosenia::getActors(world), "Kerosenia"});
                    if (*showPerspective) actorTypes.push_back({Perspective::getActors(world), "Perspective"});
                    if (*showWatcher) actorTypes.push_back({Watcher::getActors(world), "Watcher"});
                    if (*showShroom) actorTypes.push_back({Shroom::getActors(world), "Shroom"});
                    if (*showDandelion) actorTypes.push_back({Dandelion::getActors(world), "Dandelion"});
                    if (*showChowchow) actorTypes.push_back({Chowchow::getActors(world), "Chowchow"});
                    if (*showSponge) actorTypes.push_back({Sponge::getActors(world), "Sponge"});
                    if (*showParticleFish) actorTypes.push_back({ParticleFish::getActors(world), "ParticleFish"});
                    if (*showFishBaiter) actorTypes.push_back({FishBaiter::getActors(world), "FishBaiter"});

                    try {
                        SDK::AQRSLPlayerCharacter* character = Globals::getCharacter();
                        SDK::FVector characterLocation;

                        if (character) {
                            characterLocation = character->K2_GetActorLocation();
                        }

                        for (const auto& typeInfo : actorTypes) {
                            for (auto& actor : typeInfo.actors) {
                                if (actor) {
                                    try {
                                        SDK::FVector location;
                                        SDK::FVector extent;

                                        actor->GetActorBounds(true, &location, &extent, false);

                                        if (location == SDK::FVector(0, 0, 0)) {
                                            location = actor->K2_GetActorLocation();

                                            if (typeInfo.type == "Shroom" || typeInfo.type == "Nucleus") {
                                                // need to add offsets based on actor type
                                            }
                                        } else {
                                            if (typeInfo.type == "Box") {
                                                location.Z += extent.Z;
                                            }
                                        }

                                        if (!*disableDistanceLimit && character) {
                                            float distance = characterLocation.GetDistanceToInMeters(location);
                                            if (distance > *maxDistance) {
                                                continue;
                                            }
                                        }

                                        scanningActorData.locations.push_back(location);
                                        scanningActorData.names.push_back(SDK::UKismetStringLibrary::Conv_NameToString(actor->Name));
                                        scanningActorData.types.push_back(typeInfo.type);
                                    }
                                    catch (...) {
                                        // skip if exec fails
                                        continue;
                                    }
                                }
                            }
                        }

                        {
                            std::unique_lock<std::shared_mutex> lock(actorMutex);
                            currentActorData.locations.swap(scanningActorData.locations);
                            currentActorData.names.swap(scanningActorData.names);
                            currentActorData.types.swap(scanningActorData.types);
                        }
                    }
                    catch (const std::exception& e) {
                        // log if u want
                        continue;
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

        void drawBoundingBox(SDK::UCanvas* canvas, const SDK::FVector2D& screenLocation, float size) {
            const float halfSize = size / 2.0f;
            
            SDK::FVector2D topLeft(screenLocation.X - halfSize, screenLocation.Y - halfSize);
            SDK::FVector2D topRight(screenLocation.X + halfSize, screenLocation.Y - halfSize);
            SDK::FVector2D bottomLeft(screenLocation.X - halfSize, screenLocation.Y + halfSize);
            SDK::FVector2D bottomRight(screenLocation.X + halfSize, screenLocation.Y + halfSize);
            
            auto color = SDK::FLinearColor(
                (*boxColor)[0],
                (*boxColor)[1],
                (*boxColor)[2],
                (*boxColor)[3]
            );
            
            canvas->K2_DrawLine(topLeft, topRight, *outlineWidth, color);
            canvas->K2_DrawLine(topRight, bottomRight, *outlineWidth, color);
            canvas->K2_DrawLine(bottomRight, bottomLeft, *outlineWidth, color);
            canvas->K2_DrawLine(bottomLeft, topLeft, *outlineWidth, color);
        }

        void hRenderCanvas(SDK::UGameViewportClient* viewport, SDK::UCanvas* canvas) {
            // call original func first
            if (oRenderCanvas) {
                oRenderCanvas(viewport, canvas);
            }

            // render
            if (*enabled && canvas) {
                SDK::APlayerController* playerController = Globals::getPlayerController();
                if (!playerController) {
                    return;
                }

                SDK::AQRSLPlayerCharacter* character = Globals::getCharacter();
                if (!character) {
                    return;
                }

                if (!font) {
                    font = SDK::UObject::FindObject<SDK::UFont>("Font RobotoDistanceField.RobotoDistanceField");
                    if (!font) {
                        return;
                    }
                }

                float screenW  = canvas->ClipX;
                float screenH  = canvas->ClipY;
                float originX  = canvas->OrgX;
                float originY  = canvas->OrgY;

                SDK::FVector2D screenCenter(
                    originX + screenW * 0.5f,
                    originY + screenH * 0.5f
                );

                if (screenW <= 0 || screenH <= 0) {
                    screenW = (float)canvas->SizeX;
                    screenH = (float)canvas->SizeY;
                    screenCenter = SDK::FVector2D(screenW * 0.5f, screenH * 0.5f);
                }

                SDK::FVector characterLocation = character->K2_GetActorLocation();
                std::vector<SDK::FVector>       locations;
                std::vector<SDK::FString>       names;
                std::vector<std::string>        types;
                {
                    std::shared_lock<std::shared_mutex> lock(actorMutex);
                    locations = currentActorData.locations;
                    names     = currentActorData.names;
                    types     = currentActorData.types;
                }

                for (size_t i = 0; i < locations.size(); i++) {
                    if (!shouldShowActorType(types[i])) continue;

                    SDK::FVector2D screenLoc;
                    if (!playerController->ProjectWorldLocationToScreen(locations[i], &screenLoc, false))
                        continue;

                    if (*drawTracer) {
                        auto tc = SDK::FLinearColor(
                            (*tracerColor)[0], (*tracerColor)[1],
                            (*tracerColor)[2], (*tracerColor)[3]
                        );
                        canvas->K2_DrawLine(screenCenter, screenLoc, *tracerWidth, tc);
                    }

                    if (*drawBox) {
                        drawBoundingBox(canvas, screenLoc, 30.0f);
                    }

                    if (*drawName) {
                        auto txtC = SDK::FLinearColor(
                            (*textColor)[0], (*textColor)[1],
                            (*textColor)[2], (*textColor)[3]
                        );
                        canvas->K2_DrawText(
                            font, names[i], screenLoc,
                            SDK::FVector2D(*fontSize, *fontSize),
                            txtC, 1.0f,
                            SDK::FLinearColor(0,0,0,1), SDK::FVector2D(0,0),
                            true, true, true, SDK::FLinearColor(0,0,0,1)
                        );
                    }
                    
                    if (*drawDistance) {
                        float dist = characterLocation.GetDistanceToInMeters(locations[i]);
                        wchar_t buf[64];
                        swprintf(buf, 64, L"%.2fm", dist);
                        SDK::FString ds = SDK::UKismetStringLibrary::Conv_NameToString(
                            SDK::UKismetStringLibrary::Conv_StringToName(buf)
                        );
                        SDK::FVector2D dpos = screenLoc;
                        dpos.Y += font->EmScale * (*fontSize) * 0.5f;
                        auto txtC = SDK::FLinearColor(
                            (*textColor)[0], (*textColor)[1],
                            (*textColor)[2], (*textColor)[3]
                        );
                        canvas->K2_DrawText(
                            font, ds, dpos,
                            SDK::FVector2D(*fontSize, *fontSize),
                            txtC, 1.0f,
                            SDK::FLinearColor(0,0,0,1), SDK::FVector2D(0,0),
                            true, true, true, SDK::FLinearColor(0,0,0,1)
                        );
                    }
                }
            }
        }

        bool installHook() {
            if (hookInstalled) {
                return true;
            }
            
            SDK::UHottaGameEngine* engine = Globals::getEngine();
            if (!engine) {
                return false;
            }
            
            SDK::UGameViewportClient* viewport = engine->GameViewport;
            if (!viewport || !viewport->VTable) {
                return false;
            }
            
            uintptr_t addr = *(uintptr_t*)((uintptr_t)(viewport->VTable) + 0x318);
            if (!addr) {
                return false;
            }
            
            if (MH_CreateHook((LPVOID)addr, hRenderCanvas, (LPVOID*)&oRenderCanvas) != MH_OK) {
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
            
            SDK::UHottaGameEngine* engine = Globals::getEngine();
            if (!engine) {
                return false;
            }
            
            SDK::UGameViewportClient* viewport = engine->GameViewport;
            if (!viewport || !viewport->VTable) {
                return false;
            }
            
            uintptr_t addr = *(uintptr_t*)((uintptr_t)(viewport->VTable) + 0x318);
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
        
        void resetToDefaults() {
            enabled = false;
            scanDelay = 1000;
            
            drawBox = true;
            drawTracer = true;
            drawName = true;
            drawDistance = true;
            
            fontSize = 1.0f;
            outlineWidth = 1.0f;
            tracerWidth = 1.0f;
            transparency = 1.0f;
            
            boxColor = {1.0f, 1.0f, 1.0f, 1.0f};  // White
            tracerColor = {1.0f, 1.0f, 1.0f, 1.0f}; // White
            textColor = {1.0f, 1.0f, 1.0f, 1.0f};   // White

            disableDistanceLimit = false;
            maxDistance = 150.0f;
            
            showSupplyBox = true;
            showNucleus = true;
            showKerosenia = true;
            showPerspective = true;
            showWatcher = true;
            showShroom = true;
            showDandelion = true;
            showChowchow = true;
            showSponge = true;
            showParticleFish = true;
            showFishBaiter = true;
        }

        void init() {
            enabled = Config::get<bool>(confEnabled, false);
            scanDelay = Config::get<size_t>(confScanDelay, 1000);
            disableDistanceLimit = Config::get<bool>(confdisableDistanceLimit, false);
            maxDistance = Config::get<float>(confMaxDistance, 150.0f);
            
            drawBox = Config::get<bool>(confDrawBox, true);
            drawTracer = Config::get<bool>(confDrawTracer, true);
            drawName = Config::get<bool>(confDrawName, true);
            drawDistance = Config::get<bool>(confDrawDistance, true);
            
            fontSize = Config::get<float>(confFontSize, 1.0f);
            outlineWidth = Config::get<float>(confOutlineWidth, 1.0f);
            tracerWidth = Config::get<float>(confTracerWidth, 1.0f);
            transparency = Config::get<float>(confTransparency, 1.0f);
            
            std::array<float, 4> defaultBoxColor = {1.0f, 1.0f, 1.0f, 1.0f};
            std::array<float, 4> defaultTracerColor = {1.0f, 1.0f, 1.0f, 1.0f};
            std::array<float, 4> defaultTextColor = {1.0f, 1.0f, 1.0f, 1.0f};
            
            boxColor = Config::get<std::array<float, 4>>(confBoxColor, defaultBoxColor);
            tracerColor = Config::get<std::array<float, 4>>(confTracerColor, defaultTracerColor);
            textColor = Config::get<std::array<float, 4>>(confTextColor, defaultTextColor);
            
            showSupplyBox = Config::get<bool>(confShowSupplyBox, true);
            showNucleus = Config::get<bool>(confShowNucleus, true);
            showKerosenia = Config::get<bool>(confShowKerosenia, true);
            showPerspective = Config::get<bool>(confShowPerspective, true);
            showWatcher = Config::get<bool>(confShowWatcher, true);
            showShroom = Config::get<bool>(confShowShroom, true);
            showDandelion = Config::get<bool>(confShowDandelion, true);
            showChowchow = Config::get<bool>(confShowChowchow, true);
            showSponge = Config::get<bool>(confShowSponge, true);
            showParticleFish = Config::get<bool>(confShowParticleFish, true);
            showFishBaiter = Config::get<bool>(confShowFishBaiter, true);
            
            font = SDK::UObject::FindObject<SDK::UFont>("Font RobotoDistanceField.RobotoDistanceField");
            
            ForceAlwaysRelevantClasses();
            
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

            ImGui::Checkbox("Highest Possible Distance", &disableDistanceLimit);
            ImGui::SameLine();
            HelpMarker("If enabled, objects will be shown regardless of distance. Disables the Maximum Distance slider.");

            if (*disableDistanceLimit) {
                maxDistance = (std::numeric_limits<float>::max)();
                ImGui::BeginDisabled();
                ImGui::SliderFloat("Max Distance (meters)", &maxDistance, 150.0f, 1000.0f, "%.0f m");
                ImGui::EndDisabled();
            } else {
                ImGui::SliderFloat("Max Distance (meters)", &maxDistance, 150.0f, 1000.0f, "%.0f m");
            }
            
            ImGui::SameLine();
            HelpMarker("Maximum distance to show objects.");

            if (ImGui::Button("Reset To Default")) {
                resetToDefaults();
            }

            if (ImGui::CollapsingHeader("Config")) {
                ImGui::Text("Overlay Settings");

                ImGui::Checkbox("Draw Box", &drawBox);
                ImGui::SameLine(); 
                HelpMarker("Draw a box around detected objects");

                ImGui::Checkbox("Draw Tracer", &drawTracer);
                ImGui::SameLine(); 
                HelpMarker("Draw a line from your character to objects");

                ImGui::Checkbox("Draw Name", &drawName);
                ImGui::SameLine(); 
                HelpMarker("Display the name of objects");

                ImGui::Checkbox("Draw Distance", &drawDistance);
                ImGui::SameLine(); 
                HelpMarker("Show the distance to objects in meters");

                ImGui::Text("Font Size");
                ImGui::SameLine();
                HelpMarker("Adjust the size of the font");
                ImGui::SliderFloat("##FontSize", &fontSize, 0.5f, 2.0f);

                ImGui::Text("Outline Width");
                ImGui::SameLine();
                HelpMarker("Adjust the width of outlines");
                ImGui::SliderFloat("##OutlineWidth", &outlineWidth, 0.5f, 3.0f);

                ImGui::Text("Tracer Width");
                ImGui::SameLine();
                HelpMarker("Adjust the width of tracers");
                ImGui::SliderFloat("##TracerWidth", &tracerWidth, 0.5f, 3.0f);

                ImGui::Text("Colors");
                ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaPreview | 
                                           ImGuiColorEditFlags_NoInputs | 
                                           ImGuiColorEditFlags_DisplayRGB |
                                           ImGuiColorEditFlags_AlphaBar;

                {
                    std::array<float, 4> currentBoxColor = *boxColor;
                    float tempColor[4] = { currentBoxColor[0], currentBoxColor[1], currentBoxColor[2], currentBoxColor[3] };

                    ImGui::Text("Box Color");

                    int boxColorInt[4] = {
                        static_cast<int>(tempColor[0] * 255),
                        static_cast<int>(tempColor[1] * 255),
                        static_cast<int>(tempColor[2] * 255),
                        static_cast<int>(tempColor[3] * 255),
                    };

                    bool changed = false;
                    if (ImGui::DragInt4("##BoxColorRGB", boxColorInt, 1.0f, 0, 255)) {
                        tempColor[0] = boxColorInt[0] / 255.0f;
                        tempColor[1] = boxColorInt[1] / 255.0f;
                        tempColor[2] = boxColorInt[2] / 255.0f;
                        tempColor[3] = boxColorInt[3] / 255.0f;
                        changed = true;
                    }

                    ImGui::SameLine();
                    if (ImGui::ColorEdit4("##BoxColorPicker", tempColor, flags)) {
                        changed = true;
                    }

                    if (changed) {
                        std::array<float, 4> newColor = {
                            tempColor[0], tempColor[1], tempColor[2], tempColor[3]
                        };
                        boxColor = newColor;
                    }
                }

                {
                    std::array<float, 4> currentTracerColor = *tracerColor;
                    float tempColor[4] = { currentTracerColor[0], currentTracerColor[1], currentTracerColor[2], currentTracerColor[3] };

                    ImGui::Text("Tracer Color");

                    int tracerColorInt[4] = {
                        static_cast<int>(tempColor[0] * 255),
                        static_cast<int>(tempColor[1] * 255),
                        static_cast<int>(tempColor[2] * 255),
                        static_cast<int>(tempColor[3] * 255),
                    };

                    bool changed = false;
                    if (ImGui::DragInt4("##TracerColorRGB", tracerColorInt, 1.0f, 0, 255)) {
                        tempColor[0] = tracerColorInt[0] / 255.0f;
                        tempColor[1] = tracerColorInt[1] / 255.0f;
                        tempColor[2] = tracerColorInt[2] / 255.0f;
                        tempColor[3] = tracerColorInt[3] / 255.0f;
                        changed = true;
                    }

                    ImGui::SameLine();
                    if (ImGui::ColorEdit4("##TracerColorPicker", tempColor, flags)) {
                        changed = true;
                    }

                    if (changed) {
                        std::array<float, 4> newColor = {
                            tempColor[0], tempColor[1], tempColor[2], tempColor[3]
                        };
                        tracerColor = newColor;
                    }
                }

                {
                    std::array<float, 4> currentTextColor = *textColor;
                    float tempColor[4] = { currentTextColor[0], currentTextColor[1], currentTextColor[2], currentTextColor[3] };


                    ImGui::Text("Text Color");

                    int textColorInt[4] = {
                        static_cast<int>(tempColor[0] * 255),
                        static_cast<int>(tempColor[1] * 255),
                        static_cast<int>(tempColor[2] * 255),
                        static_cast<int>(tempColor[3] * 255),
                    };

                    bool changed = false;
                    if (ImGui::DragInt4("##TextColorRGB", textColorInt, 1.0f, 0, 255)) {
                        tempColor[0] = textColorInt[0] / 255.0f;
                        tempColor[1] = textColorInt[1] / 255.0f;
                        tempColor[2] = textColorInt[2] / 255.0f;
                        tempColor[3] = textColorInt[3] / 255.0f;
                        changed = true;
                    }

                    ImGui::SameLine();
                    if (ImGui::ColorEdit4("##TextColorPicker", tempColor, flags)) {
                        changed = true;
                    }

                    if (changed) {
                        std::array<float, 4> newColor = {
                            tempColor[0], tempColor[1], tempColor[2], tempColor[3]
                        };
                        textColor = newColor;
                    }
                }
            }

            if (ImGui::CollapsingHeader("Filters")) {
                ImGui::Checkbox("Supply Box", &showSupplyBox);
                ImGui::Checkbox("Nucleus", &showNucleus);
                ImGui::Checkbox("Kerosenia", &showKerosenia);
                ImGui::Checkbox("Perspective", &showPerspective);
                ImGui::Checkbox("Watcher", &showWatcher);
                ImGui::Checkbox("Shroom", &showShroom);
                ImGui::Checkbox("Dandelion", &showDandelion);
                ImGui::Checkbox("Chowchow", &showChowchow);
                ImGui::Checkbox("Sponge", &showSponge);
                ImGui::Checkbox("Particle Fish", &showParticleFish);
                ImGui::Checkbox("Fish Baiter", &showFishBaiter);
            }
        }
    } // namespace Esp
} // namespace Feats