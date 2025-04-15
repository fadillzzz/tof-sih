#include "shroom.hpp"

namespace Feats {
    namespace Esp {
        namespace Shroom {
            template <typename T>
            void scanActors(SDK::UWorld* world, std::vector<SDK::TWeakObjectPtr<SDK::AActor>>& actors, SDK::UClass* classType) {
                SDK::TArray<SDK::AActor*> shrooms;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, T::StaticClass(), &shrooms);

                for (SDK::AActor* actor : shrooms) {
                    if (!actor)
                       continue;
                    T* shroom = static_cast<T*>(actor);
                    uint32_t unkFlag = *(uint32_t*)((uint8_t*)shroom + 0x1C0);
                    if (unkFlag == 5)
                       continue;
                    actors.push_back(SDK::TWeakObjectPtr<SDK::AActor>(shroom));
                }
            }

            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actors;
                scanActors<SDK::ABP_Manager_LumenMushroom_C>(world, actors, SDK::ABP_Manager_LumenMushroom_C::StaticClass());
                scanActors<SDK::ABP_LitMushroom_Manager_C>(world, actors, SDK::ABP_LitMushroom_Manager_C::StaticClass());
                return actors;
            }
        } // namespace Shroom
    } // namespace Esp
} // namespace Feats