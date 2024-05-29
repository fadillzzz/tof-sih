#include "dandelion.hpp"

namespace Feats {
    namespace Esp {
        namespace Dandelion {
            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<std::shared_ptr<SDK::AActor>> actors;

                SDK::TArray<SDK::ABP_MiniGame_FlyFlower_001_C *> dandelions;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_MiniGame_FlyFlower_001_C::StaticClass(),
                                                           (SDK::TArray<SDK::AActor *> *)&dandelions);

                for (auto &dandelion : dandelions) {
                    const auto unkFlag = *(uint32_t *)((uint8_t *)dandelion + 0x1C0);

                    if (unkFlag != 0x1FFFFFF) {
                        continue;
                    }

                    actors.push_back(std::make_shared<SDK::ABP_MiniGame_FlyFlower_001_C>(*dandelion));
                }

                return actors;
            }
        } // namespace Dandelion
    } // namespace Esp
} // namespace Feats