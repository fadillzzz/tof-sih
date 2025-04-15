#include "sponge.hpp"

namespace Feats {
    namespace Esp {
        namespace Sponge {
            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actors;
                SDK::TArray<SDK::ABP_SeaSponge_Base_C*> spongeActors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_SeaSponge_Base_C::StaticClass(),
                                                           (SDK::TArray<SDK::AActor*>*)&spongeActors);

                for (SDK::ABP_SeaSponge_Base_C* sponge : spongeActors) {
                    if (!sponge)
                       continue;
                    uint32_t unkFlag = *(uint32_t*)((uint8_t*)sponge + 0x1C0) & 0x0000000F;
                    if (unkFlag == 0xD)
                        continue;
                    actors.push_back(SDK::TWeakObjectPtr<SDK::AActor>(sponge));
                }
                return actors;
            }
        } // namespace Sponge
    } // namespace Esp
} // namespace Feats