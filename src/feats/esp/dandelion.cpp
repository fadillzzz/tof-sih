#include "dandelion.hpp"

namespace Feats {
    namespace Esp {
        namespace Dandelion {
            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actors;

                SDK::TArray<SDK::ABP_MiniGame_FlyFlower_001_C*> dandelions;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_MiniGame_FlyFlower_001_C::StaticClass(),
                                                           (SDK::TArray<SDK::AActor*>*)&dandelions);

                for (SDK::ABP_MiniGame_FlyFlower_001_C* dandelion : dandelions) {
                    if (!dandelion)
                        continue;
                    uint32_t unkFlag = *(uint32_t*)((uint8_t*)dandelion + 0x1C0);
                    if (unkFlag != 0x1FFFFFF)
                        continue;
                    actors.push_back(SDK::TWeakObjectPtr<SDK::AActor>(dandelion));
                }
                return actors;
            }
        } // namespace Dandelion
    } // namespace Esp
} // namespace Feats