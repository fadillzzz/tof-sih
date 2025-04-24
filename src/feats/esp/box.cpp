#include "box.hpp"
#include "../../globals.hpp"

namespace Feats {
    namespace Esp {
        namespace Box {

            std::vector<SDK::TWeakObjectPtr<SDK::AActor>> getActors(SDK::UWorld *world) {
                std::vector<SDK::TWeakObjectPtr<SDK::AActor>> actorsCopy;
                
                // Get all treasure box actors
                auto boxes = Globals::getAllObjects<SDK::AQRSLTreasureBoxActor *>(SDK::AQRSLTreasureBoxActor::StaticClass());
                
                const bool isMirroria = (world->GetName() == "Vera_city");
                
                // For Vera_city (Mirroria), filter special cases
                if (isMirroria) {
                    boxes.erase(std::remove_if(boxes.begin(), boxes.end(),
                        [](SDK::AQRSLTreasureBoxActor *box) {
                            if (box->CanOpenParticle.WeakPtr.ObjectIndex == UINT_MAX &&
                                box->CanOpenParticle.WeakPtr.ObjectSerialNumber == 0) {
                                return true;
                            }
                            return false;
                        }),
                    boxes.end());
                } else {
                    // For non-Mirroria maps, only include "OnceOnly" boxes (supply boxes)
                    boxes.erase(std::remove_if(boxes.begin(), boxes.end(),
                        [](SDK::AQRSLTreasureBoxActor *box) {
                            // Keep only "OnceOnly" boxes which are the supply boxes
                            if (!box->GetName().contains("OnceOnly")) {
                                return true;
                            }
                            return false;
                        }),
                    boxes.end());
                }
                
                // Further filter remaining boxes
                for (auto* box : boxes) {
                    if (!box)
                        continue;
                        
                    // Skip harvested boxes
                    if (box->bHarvested)
                        continue;
                        
                    // Skip if specific flag is set (using the same check as original code)
                    if (*((uint8_t*)box + 0xDC0) == 1)
                        continue;
                        
                    // Skip based on PlayNotOpenParticle
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