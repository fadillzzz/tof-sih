#include "sponge.hpp"

namespace Feats {
    namespace Esp {
        namespace Sponge {
            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<std::shared_ptr<SDK::AActor>> actors;

                SDK::TArray<SDK::ABP_SeaSponge_Base_C *> spongeActors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_SeaSponge_Base_C::StaticClass(),
                                                           (SDK::TArray<SDK::AActor *> *)&spongeActors);

                for (const auto &spongeActor : spongeActors) {
                    const auto unkFlag = *(uint32_t *)((uint8_t *)spongeActor + 0x1C0) & 0x0000000F;

                    if (unkFlag == 0xD) {
                        continue;
                    }

                    actors.push_back(std::make_shared<SDK::AActor>(*spongeActor));
                }

                return actors;
            }
        } // namespace Sponge
    } // namespace Esp
} // namespace Feats