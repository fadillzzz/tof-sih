#include "esp.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "minhook/include/MinHook.h"

namespace Feats {
    namespace Esp {
        typedef void (*RenderCanvas)(SDK::UGameViewportClient *, SDK::UCanvas *);
        RenderCanvas oRenderCanvas = nullptr;

        std::chrono::time_point<std::chrono::steady_clock> lastScan = std::chrono::steady_clock::now();
        SDK::TArray<SDK::AHottaVisualActor *> actors;

        SDK::UFont *font = SDK::UObject::FindObject<SDK::UFont>("Font RobotoDistanceField.RobotoDistanceField");

        SDK::TArray<SDK::AHottaVisualActor *> getActors() {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastScan)
                    .count() < 500) {
                return actors;
            }

            actors.Clear();
            SDK::UGameplayStatics::GetAllActorsOfClass(Globals::getWorld(), SDK::AHottaVisualActor::StaticClass(),
                                                       (SDK::TArray<SDK::AActor *> *)&actors);

            return actors;
        }

        void hRenderCanvas(SDK::UGameViewportClient *viewport, SDK::UCanvas *canvas) {
            const auto character = Globals::getCharacter();

            if (character != nullptr) {
                const auto actors = getActors();
                const auto playerController = character->GetHottaPlayerController();
                SDK::FVector2D characterScreenLocation;

                playerController->ProjectWorldLocationToScreen(character->K2_GetActorLocation(),
                                                               &characterScreenLocation, false);

                for (size_t i = 0; i < actors.Num(); i++) {
                    const auto actor = actors[i];

                    SDK::FVector2D screenLocation;
                    if (playerController->ProjectWorldLocationToScreen(actor->K2_GetActorLocation(), &screenLocation,
                                                                       false)) {
                        canvas->K2_DrawLine(characterScreenLocation, screenLocation, 1.0f,
                                            SDK::FLinearColor(255, 0, 0, 255));

                        canvas->K2_DrawText(font, SDK::UKismetTextLibrary::Conv_TextToString(actor->ActorName),
                                            screenLocation, SDK::FVector2D(0.75f, 0.75f),
                                            SDK::FLinearColor(255, 0, 0, 255), 1.25f, SDK::FLinearColor(0, 0, 0, 255),
                                            SDK::FVector2D(0, 0), true, true, true,
                                            SDK::FLinearColor(255, 255, 255, 255));
                    }
                }
            }

            oRenderCanvas(viewport, canvas);
        }

        void init() {
            const auto engine = Globals::getEngine();
            const auto viewport = engine->GameViewport;

            if (viewport != nullptr) {
                const auto addr = *(uintptr_t *)((uintptr_t)(viewport->VTable) + 0x318);
                MH_CreateHook((LPVOID)addr, hRenderCanvas, (LPVOID *)&oRenderCanvas);
                MH_EnableHook((LPVOID)addr);
            }
        }

        void shutdown() {
            const auto engine = Globals::getEngine();
            const auto viewport = engine->GameViewport;

            if (viewport != nullptr) {
                const auto addr = *(uintptr_t *)((uintptr_t)(viewport->VTable) + 0x318);
                MH_DisableHook((LPVOID)addr);
                MH_RemoveHook((LPVOID)addr);
            }
        }

        void tick() { return; }

        void menu() { return; }
    } // namespace Esp
} // namespace Feats