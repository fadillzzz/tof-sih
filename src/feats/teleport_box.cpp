#include "teleport_box.hpp"
#include "../globals.hpp"
#include "hotkey.hpp"

namespace Feats {
    namespace TeleportBox {
        Config::field<bool> includeRespawn;

        void init() { includeRespawn = Config::get<bool>(confIncludeRespawn, false); }

        void teleport() {
            const auto character = Globals::getCharacter();

            if (character != nullptr) {
                auto boxes =
                    Globals::getAllObjects<SDK::AQRSLTreasureBoxActor *>(SDK::AQRSLTreasureBoxActor::StaticClass());

                boxes.erase(std::remove_if(boxes.begin(), boxes.end(),
                                           [](SDK::AQRSLTreasureBoxActor *box) {
                                               if (box->bHarvested == true) {
                                                   return true;
                                               }

                                               if (*((uint8_t *)box + 0xDB0) == 1) {
                                                   return true;
                                               }

                                               if (*includeRespawn == false) {
                                                   if (!box->GetName().contains("OnceOnly")) {
                                                       return true;
                                                   }
                                               }

                                               return false;
                                           }),
                            boxes.end());

                if (boxes.size() > 0) {
                    auto closest = std::min_element(boxes.begin(), boxes.end(), [&](auto *a, auto *b) -> bool {
                        auto aPos = a->K2_GetActorLocation();
                        auto bPos = b->K2_GetActorLocation();
                        float distA = (aPos == SDK::FVector(0, 0, 0)) ? (std::numeric_limits<float>::max)()
                                                                      : character->GetDistanceTo(a);
                        float distB = (bPos == SDK::FVector(0, 0, 0)) ? (std::numeric_limits<float>::max)()
                                                                      : character->GetDistanceTo(b);
                        return distA < distB;
                    });

                    if (closest != boxes.end()) {
                        auto *closestActor = *closest;
                        SDK::FVector newPos;
                        SDK::FVector extent;
                        closestActor->GetActorBounds(true, &newPos, &extent, false);
                        newPos.Z += extent.Z;
                        const auto location =
                            (SDK::FVector *)((byte *)character->CharacterMovement->UpdatedComponent + 0x1E0);
                        *location = newPos;
                    }
                }
            }
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confActivate)) {
                teleport();
            }
        }

        void menu() {
            if (ImGui::Button("Teleport to closest box")) {
                teleport();
            }

            ImGui::SameLine();

            ImGui::Checkbox("Includes respawning boxes", &includeRespawn);
        }
    } // namespace TeleportBox
} // namespace Feats