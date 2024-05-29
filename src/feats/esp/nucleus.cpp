#include "nucleus.hpp"

namespace Feats {
    namespace Esp {
        namespace Nucleus {
            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                const auto isMirroria = world->GetName() == "Vera_city";

                SDK::TArray<SDK::AActor *> actors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world,
                                                           isMirroria ? SDK::AVeraCity_Gem_BP_C::StaticClass()
                                                                      : SDK::ABP_Harvest_Gem_Base_C::StaticClass(),
                                                           &actors);

                std::vector<std::shared_ptr<SDK::AActor>> actorsCopy;

                for (size_t i = 0; i < actors.Num(); i++) {
                    const auto nucleus = actors[i];

                    if (isMirroria) {
                        const auto gem = static_cast<SDK::AVeraCity_Gem_BP_C *>(nucleus);

                        if (gem->Overlap->GetCollisionEnabled() == SDK::ECollisionEnabled::NoCollision) {
                            continue;
                        }
                    }

                    const auto nucleusPtr = std::make_shared<SDK::AActor>(*nucleus);
                    actorsCopy.push_back(nucleusPtr);
                }

                return actorsCopy;
            }
        } // namespace Nucleus
    } // namespace Esp
} // namespace Feats