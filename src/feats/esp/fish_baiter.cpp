#include "fish_baiter.hpp"

namespace Feats {
    namespace Esp {
        namespace FishBaiter {
            std::vector<std::shared_ptr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<std::shared_ptr<SDK::AActor>> actors;

                SDK::TArray<SDK::ABP_FishBall_Base_C *> fishBallActors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_FishBall_Base_C::StaticClass(),
                                                           (SDK::TArray<SDK::AActor *> *)&fishBallActors);

                for (auto &fishBallActor : fishBallActors) {
                    if (fishBallActor->Did0Finished && fishBallActor->Did1Finished && fishBallActor->Did2Finished &&
                        fishBallActor->Yahaha_SpawnManager->Children.Num() == 0) {
                        continue;
                    }

                    actors.push_back(std::make_shared<SDK::AActor>(*fishBallActor));
                }

                return actors;
            }
        } // namespace FishBaiter
    } // namespace Esp
} // namespace Feats
