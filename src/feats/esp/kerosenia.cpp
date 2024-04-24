#include "kerosenia.hpp"

namespace Feats {
    namespace Esp {
        namespace Kerosenia {
            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                SDK::TArray<SDK::AActor *> actors;

                if (world->GetName().contains("Vera") || world->GetName().contains("Map_sea")) {
                    SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_MiniGame_FireLink_Vera_C::StaticClass(),
                                                               &actors);
                } else {
                    SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_Fire_MiniGame_BPBase_C::StaticClass(),
                                                               &actors);
                }

                std::vector<std::shared_ptr<SDK::AActor>> actorsCopy;
                for (size_t i = 0; i < actors.Num(); i++) {
                    const auto kerosenia = (SDK::AHottaFireRelatedActor *)actors[i];

                    if (kerosenia->ActorReplicatedInfo.bInBurnState) {
                        continue;
                    }

                    const auto unkFlag = *(uint32_t *)((uint8_t *)kerosenia + 0x1C0);

                    if (unkFlag == 0xFFF) {
                        continue;
                    }

                    const auto keroseniaPtr = std::make_shared<SDK::AHottaFireRelatedActor>(*kerosenia);
                    actorsCopy.push_back(keroseniaPtr);
                }

                return actorsCopy;
            }
        } // namespace Kerosenia
    } // namespace Esp
} // namespace Feats
