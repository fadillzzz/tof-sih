#include "fish_baiter.hpp"

namespace Feats {
    namespace Esp {
        namespace FishBaiter {
            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actors;

                SDK::TArray<SDK::ABP_FishBall_Base_C*> fishBallActors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::ABP_FishBall_Base_C::StaticClass(),
                                                           (SDK::TArray<SDK::AActor*>*)&fishBallActors);

                for (SDK::ABP_FishBall_Base_C* fishBall : fishBallActors) {
                    if (!fishBall)
                        continue;
                    if (fishBall->Did0Finished && fishBall->Did1Finished && fishBall->Did2Finished &&
                        fishBall->Yahaha_SpawnManager->Children.Num() == 0)
                        continue;
                    actors.push_back(SDK::TWeakObjectPtr<SDK::AActor>(fishBall));
                }
                return actors;
            }
        } // namespace FishBaiter
    } // namespace Esp
} // namespace Feats