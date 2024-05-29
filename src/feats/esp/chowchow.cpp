#include "chowchow.hpp"

namespace Feats {
    namespace Esp {
        namespace Chowchow {
            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<std::shared_ptr<SDK::AActor>> actors;

                SDK::TArray<SDK::AActor *> chowchows;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_MiniGame_ThrowFlower_002_C::StaticClass(),
                                                           &chowchows);

                for (const auto &chowchow : chowchows) {
                    const auto unkFlag = *(uint32_t *)((uint8_t *)chowchow + 0x1C0);

                    if (unkFlag != 0xFF) {
                        continue;
                    }

                    actors.push_back(std::make_shared<SDK::AActor>(*chowchow));
                }

                return actors;
            }
        } // namespace Chowchow
    } // namespace Esp
} // namespace Feats
