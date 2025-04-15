#include "perspective.hpp"

namespace Feats {
    namespace Esp {
        namespace Perspective {
            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                SDK::TArray<SDK::AActor*> actors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_Minigame_PerspectivePuzzle_Base_C::StaticClass(), &actors);

                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actorsCopy;
                for (SDK::AActor* actor : actors) {
                    if (!actor)
                       continue;
                    SDK::ABP_Minigame_PerspectivePuzzle_Base_C* perspective = static_cast<SDK::ABP_Minigame_PerspectivePuzzle_Base_C*>(actor);
                    if (perspective->IsSucceed)
                       continue;
                    actorsCopy.push_back(SDK::TWeakObjectPtr<SDK::AActor>(actor));
                }

                const bool isInnars = world->GetName().contains("Map_sea");
                if (isInnars) {
                    SDK::TArray<SDK::AActor*> seaActors;
                    SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_Minigame_PerspectivePuzzle_Sea_Base_C::StaticClass(), &seaActors);
                    for (SDK::AActor* actor : seaActors) {
                        if (!actor)
                           continue;
                        SDK::ABP_Minigame_PerspectivePuzzle_Sea_Base_C* perspective = static_cast<SDK::ABP_Minigame_PerspectivePuzzle_Sea_Base_C*>(actor);
                        if (perspective->IsSucceed)
                           continue;
                        actorsCopy.push_back(SDK::TWeakObjectPtr<SDK::AActor>(actor));
                    }
                }
                return actorsCopy;
            }
        } // namespace Perspective
    } // namespace Esp
} // namespace Feats