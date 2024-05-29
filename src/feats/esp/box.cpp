#include "box.hpp"

namespace Feats {
    namespace Esp {
        namespace Box {
            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                SDK::TArray<SDK::AActor *> actors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::AQRSLTreasureBoxActor::StaticClass(),
                                                           (SDK::TArray<SDK::AActor *> *)&actors);

                std::vector<std::shared_ptr<SDK::AActor>> actorsCopy;
                const auto isMirroria = world->GetName() == "Vera_city";

                for (size_t i = 0; i < actors.Num(); i++) {
                    const auto box = (SDK::AQRSLTreasureBoxActor *)actors[i];

                    if (isMirroria) {
                        if (box->CanOpenParticle.WeakPtr.ObjectIndex == UINT_MAX &&
                            box->CanOpenParticle.WeakPtr.ObjectSerialNumber == 0) {
                            continue;
                        }
                    }

                    if (box->bHarvested == true) {
                        continue;
                    }

                    if (*((uint8_t *)box + 0xDC0) == 1) {
                        continue;
                    }

                    const auto boxPtr = std::make_shared<SDK::AQRSLTreasureBoxActor>(*box);
                    actorsCopy.push_back(boxPtr);
                }

                return actorsCopy;
            }
        } // namespace Box
    } // namespace Esp
} // namespace Feats