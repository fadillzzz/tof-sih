#include "shroom.hpp"

namespace Feats {
    namespace Esp {
        namespace Shroom {
            template <typename T>
            void scanActors(SDK::UWorld *world, std::vector<std::shared_ptr<SDK::AActor>> &actors,
                            SDK::UClass *classType) {
                SDK::TArray<SDK::AActor *> shrooms;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, T::StaticClass(), &shrooms);

                for (size_t i = 0; i < shrooms.Num(); i++) {
                    const auto shroom = (T *)shrooms[i];

                    const auto unkFlag = *(uint32_t *)((uint8_t *)shroom + 0x1C0);

                    if (unkFlag == 5) {
                        continue;
                    }

                    actors.push_back(std::make_shared<SDK::AActor>(*shroom));
                }
            }

            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<std::shared_ptr<SDK::AActor>> actors;

                scanActors<SDK::ABP_Manager_LumenMushroom_C>(world, actors,
                                                             SDK::ABP_Manager_LumenMushroom_C::StaticClass());

                scanActors<SDK::ABP_LitMushroom_Manager_C>(world, actors,
                                                           SDK::ABP_LitMushroom_Manager_C::StaticClass());

                return actors;
            }
        } // namespace Shroom
    } // namespace Esp
} // namespace Feats