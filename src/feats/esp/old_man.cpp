#include "old_man.hpp"

namespace Feats {
    namespace Esp {
        namespace OldMan {
            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<std::shared_ptr<SDK::AActor>> actors;

                SDK::TArray<SDK::AActor *> oldManActors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ANpcShop_home_C::StaticClass(), &oldManActors);

                for (const auto &oldManActor : oldManActors) {
                    actors.push_back(std::make_shared<SDK::AActor>(*oldManActor));
                }

                return actors;
            }
        } // namespace OldMan
    } // namespace Esp
} // namespace Feats
