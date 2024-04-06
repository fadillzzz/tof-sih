#include "teleport_nucleus.hpp"
#include "../globals.hpp"

namespace Feats {
    namespace TeleportNucleus {
        void init() { return; }

        void tick() { return; }

        void menu() {
            if (ImGui::Button("Teleport to closest nucleus")) {
                const auto character = Globals::getCharacter();

                if (character != nullptr) {
                    const auto nuclei = Globals::getAllObjects<SDK::ABP_Harvest_Gem_Base_C *>(
                        SDK::ABP_Harvest_Gem_Base_C::StaticClass());

                    if (nuclei.size() > 0) {
                        auto closestNucleus =
                            std::min_element(nuclei.begin(), nuclei.end(), [&](auto *a, auto *b) -> bool {
                                auto aPos = a->K2_GetActorLocation();
                                auto bPos = b->K2_GetActorLocation();
                                float distA = (aPos == SDK::FVector(0, 0, 0)) ? (std::numeric_limits<float>::max)()
                                                                              : character->GetDistanceTo(a);
                                float distB = (bPos == SDK::FVector(0, 0, 0)) ? (std::numeric_limits<float>::max)()
                                                                              : character->GetDistanceTo(b);
                                return distA < distB;
                            });

                        if (closestNucleus != nuclei.end()) {
                            SDK::ABP_Harvest_Gem_Base_C *closestActor = (SDK::ABP_Harvest_Gem_Base_C *)*closestNucleus;
                            auto newPos = closestActor->K2_GetActorLocation();
                            newPos.Z += 500;
                            auto newRot = character->K2_GetActorRotation();
                            character->SafeTeleportTo(newPos, newRot);
                        }
                    }
                }
            }
        }
    } // namespace TeleportNucleus
} // namespace Feats