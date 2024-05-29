#include "particle_fish.hpp"

namespace Feats {
    namespace Esp {
        namespace ParticleFish {
            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<std::shared_ptr<SDK::AActor>> actors;

                SDK::TArray<SDK::ABP_ParticleFish_Base_C *> particleFishActors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_ParticleFish_Base_C::StaticClass(),
                                                           (SDK::TArray<SDK::AActor *> *)&particleFishActors);

                for (auto &particleFishActor : particleFishActors) {
                    const auto unkFlag = *(uint32_t *)((uint8_t *)particleFishActor + 0x1C0) & 0x0000000F;

                    if (unkFlag == 0xD) {
                        continue;
                    }

                    actors.push_back(std::make_shared<SDK::AActor>(*particleFishActor));
                }

                return actors;
            }
        } // namespace ParticleFish
    } // namespace Esp
} // namespace Feats
