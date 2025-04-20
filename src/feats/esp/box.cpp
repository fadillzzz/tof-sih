namespace Feats {
    namespace Esp {
        namespace Box {

            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                SDK::TArray<SDK::AActor*> actors;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::AQRSLTreasureBoxActor::StaticClass(), &actors);

                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actorsCopy;
                const bool isMirroria = (world->GetName() == "Vera_city");

                for (SDK::AActor* actor : actors) {
                    if (!actor)
                        continue;

                    SDK::AQRSLTreasureBoxActor* box = static_cast<SDK::AQRSLTreasureBoxActor*>(actor);
                    
                    if (box->bHarvested)
                        continue;

                    if (*((uint8_t*)box + 0xDC0) == 1)
                        continue;

                    if (*(int *)(reinterpret_cast<uint8_t *>(&(box->PlayNotOpenParticle)) + 
                                sizeof(box->PlayNotOpenParticle)) == 1)
                        continue;

                    actorsCopy.push_back(SDK::TWeakObjectPtr<SDK::AActor>(box));
                }
                return actorsCopy;
            }

        } // namespace Box
    } // namespace Esp
} // namespace Feats