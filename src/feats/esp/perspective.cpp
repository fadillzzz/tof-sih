#include "perspective.hpp"

namespace Feats {
    namespace Esp {
        namespace Perspective {
            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                SDK::TArray<SDK::AActor *> actors;

                SDK::UGameplayStatics::GetAllActorsOfClass(
                    world, SDK::ABP_Minigame_PerspectivePuzzle_Base_C::StaticClass(), &actors);

                std::vector<std::shared_ptr<SDK::AActor>> actorsCopy;

                for (size_t i = 0; i < actors.Num(); i++) {
                    const auto perspective = (SDK::ABP_Minigame_PerspectivePuzzle_Base_C *)actors[i];
                    if (perspective->IsSucceed) {
                        continue;
                    }

                    const auto perspectivePtr = std::make_shared<SDK::AHottaVisualActor>(*perspective);
                    actorsCopy.push_back(perspectivePtr);
                }

                const auto isInnars = world->GetName().contains("Map_sea");

                if (isInnars) {
                    SDK::TArray<SDK::AActor *> seaActors;
                    SDK::UGameplayStatics::GetAllActorsOfClass(
                        world, SDK::ABP_Minigame_PerspectivePuzzle_Sea_Base_C::StaticClass(), &seaActors);

                    for (size_t i = 0; i < seaActors.Num(); i++) {
                        const auto perspective = (SDK::ABP_Minigame_PerspectivePuzzle_Sea_Base_C *)seaActors[i];

                        if (perspective->IsSucceed) {
                            continue;
                        }

                        const auto perspectivePtr = std::make_shared<SDK::AHottaVisualActor>(*perspective);
                        actorsCopy.push_back(perspectivePtr);
                    }
                }

                return actorsCopy;
            }
        } // namespace Perspective
    } // namespace Esp
} // namespace Feats
