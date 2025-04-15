#include "watcher.hpp"

namespace Feats {
    namespace Esp {
        namespace Watcher {
            template <typename T>
            void scanActors(SDK::UWorld* world, std::vector<SDK::TWeakObjectPtr<SDK::AActor>>& actorsVec, SDK::UClass* classType) {
                SDK::TArray<SDK::AActor*> actors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, classType, &actors);

                for (SDK::AActor* actor : actors) {
                    if (!actor)
                        continue;
                    T* watcher = static_cast<T*>(actor);
                    if (watcher->IsFinished_)
                        continue;
                    if (!watcher->bUseBPInteractEntries)
                        continue;
                    actorsVec.push_back(SDK::TWeakObjectPtr<SDK::AActor>(watcher));
                }
            }

            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actorsCopy;
                scanActors<SDK::ABP_EternalWatcher_C>(world, actorsCopy, SDK::ABP_EternalWatcher_C::StaticClass());
                const bool isInnars = world->GetName().contains("Map_sea");
                if (isInnars) {
                    scanActors<SDK::ABP_SeaWatcher_C>(world, actorsCopy, SDK::ABP_SeaWatcher_C::StaticClass());
                }
                return actorsCopy;
            }
        } // namespace Watcher
    } // namespace Esp
} // namespace Feats