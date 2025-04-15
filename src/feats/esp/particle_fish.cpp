#include "particle_fish.hpp"

namespace Feats {
    namespace Esp {
        namespace ParticleFish {
            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actors;

                SDK::TArray<SDK::ABP_ParticleFish_Base_C*> particleFishActors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_ParticleFish_Base_C::StaticClass(),
                                                           (SDK::TArray<SDK::AActor*>*)&particleFishActors);

                for (SDK::ABP_ParticleFish_Base_C* fish : particleFishActors) {
                    if (!fish)
                       continue;
                    uint32_t unkFlag = *(uint32_t*)((uint8_t*)fish + 0x1C0) & 0x0000000F;
                    if (unkFlag == 0xD)
                       continue;
                    actors.push_back(SDK::TWeakObjectPtr<SDK::AActor>(fish));
                }
                return actors;
            }
        } // namespace ParticleFish
    } // namespace Esp
} // namespace Feats