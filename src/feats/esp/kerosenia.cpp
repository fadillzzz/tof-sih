#include "kerosenia.hpp"

namespace Feats {
    namespace Esp {
        namespace Kerosenia {
            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                SDK::TArray<SDK::AActor*> actors;

                if (world->GetName().contains("Vera") || world->GetName().contains("Map_sea"))
                    SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_MiniGame_FireLink_Vera_C::StaticClass(), &actors);
                else
                    SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_Fire_MiniGame_BPBase_C::StaticClass(), &actors);

                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actorsCopy;
                for (SDK::AActor* actor : actors) {
                    if (!actor)
                       continue;
                    SDK::AHottaFireRelatedActor* kerosenia = static_cast<SDK::AHottaFireRelatedActor*>(actor);
                    if (kerosenia->ActorReplicatedInfo.bInBurnState)
                       continue;
                    uint32_t unkFlag = *(uint32_t*)((uint8_t*)kerosenia + 0x1C0);
                    if (unkFlag == 0xFFF)
                       continue;
                    actorsCopy.push_back(SDK::TWeakObjectPtr<SDK::AActor>(kerosenia));
                }
                return actorsCopy;
            }
        } // namespace Kerosenia
    } // namespace Esp
} // namespace Feats