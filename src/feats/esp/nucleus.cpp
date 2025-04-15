#include "nucleus.hpp"

namespace Feats {
    namespace Esp {
        namespace Nucleus {
            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                const bool isMirroria = (world->GetName() == "Vera_city");

                SDK::TArray<SDK::AActor*> actors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world,
                    isMirroria ? SDK::AVeraCity_Gem_BP_C::StaticClass() : SDK::ABP_Harvest_Gem_Base_C::StaticClass(),
                    &actors);

                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actorsCopy;
                for (SDK::AActor* actor : actors) {
                    if (!actor)
                       continue;
                    if (isMirroria) {
                        SDK::AVeraCity_Gem_BP_C* gem = static_cast<SDK::AVeraCity_Gem_BP_C*>(actor);
                        if (gem->Overlap->GetCollisionEnabled() == SDK::ECollisionEnabled::NoCollision)
                            continue;
                    }
                    actorsCopy.push_back(SDK::TWeakObjectPtr<SDK::AActor>(actor));
                }
                return actorsCopy;
            }
        } // namespace Nucleus
    } // namespace Esp
} // namespace Feats