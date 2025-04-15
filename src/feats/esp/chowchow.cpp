#include "chowchow.hpp"

namespace Feats {
    namespace Esp {
        namespace Chowchow {
            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actors;

                SDK::TArray<SDK::AActor*> chowchows;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_MiniGame_ThrowFlower_002_C::StaticClass(), &chowchows);

                for (SDK::AActor* actor : chowchows) {
                    if (!actor)
                       continue;
                    uint32_t unkFlag = *(uint32_t*)((uint8_t*)actor + 0x1C0);
                    if (unkFlag != 0xFF)
                        continue;
                    actors.push_back(SDK::TWeakObjectPtr<SDK::AActor>(actor));
                }
                return actors;
            }
        } // namespace Chowchow
    } // namespace Esp
} // namespace Feats