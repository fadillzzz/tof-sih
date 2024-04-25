#include "watcher.hpp"

namespace Feats {
    namespace Esp {
        namespace Watcher {
            template <typename T>
            void scanActors(SDK::UWorld *world, std::vector<std::shared_ptr<SDK::AActor>> &actorsVec,
                            SDK::UClass *classType) {
                SDK::TArray<SDK::AActor *> actors;

                SDK::UGameplayStatics::GetAllActorsOfClass(world, classType, &actors);

                for (size_t i = 0; i < actors.Num(); i++) {
                    const auto actor = (T *)actors[i];

                    if (actor->IsFinished_) {
                        continue;
                    }

                    if (!actor->bUseBPInteractEntries) {
                        continue;
                    }

                    const auto actorPtr = std::make_shared<T>(*actor);
                    actorsVec.push_back(actorPtr);
                }
            }

            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<std::shared_ptr<SDK::AActor>> actorsCopy;

                scanActors<SDK::ABP_EternalWatcher_C>(world, actorsCopy, SDK::ABP_EternalWatcher_C::StaticClass());

                const auto isInnars = world->GetName().contains("Map_sea");

                if (isInnars) {
                    scanActors<SDK::ABP_SeaWatcher_C>(world, actorsCopy, SDK::ABP_SeaWatcher_C::StaticClass());
                }

                return actorsCopy;
            }
        } // namespace Watcher
    } // namespace Esp
} // namespace Feats
