#include "teleport_nucleus.hpp"
#include "../globals.hpp"
#include "../logger/logger.hpp"
#include "hotkey.hpp"

namespace Feats {
    namespace TeleportNucleus {
        void init() { return; }

        void teleport() {
            const auto character = Globals::getCharacter();

            if (character != nullptr) {
                const auto world = Globals::getWorld();

                auto nuclei = Globals::getAllObjects<SDK::AActor *>(world->GetName() == "Vera_city"
                                                                        ? SDK::AVeraCity_Gem_BP_C::StaticClass()
                                                                        : SDK::ABP_Harvest_Gem_Base_C::StaticClass());

                if (world->GetName() == "Vera_city") {
                    // Mirroria gems do not get removed from the world when they are collected.
                    // Instead they just go invisible and have collision disabled.
                    nuclei.erase(std::remove_if(nuclei.begin(), nuclei.end(),
                                                [&](auto *nucleus) -> bool {
                                                    const auto *gem = static_cast<SDK::AVeraCity_Gem_BP_C *>(nucleus);
                                                    return gem->Overlap->GetCollisionEnabled() ==
                                                           SDK::ECollisionEnabled::NoCollision;
                                                }),
                                 nuclei.end());
                }

                if (nuclei.size() > 0) {
                    auto closestNucleus = std::min_element(nuclei.begin(), nuclei.end(), [&](auto *a, auto *b) -> bool {
                        auto aPos = a->K2_GetActorLocation();
                        auto bPos = b->K2_GetActorLocation();
                        float distA = (aPos == SDK::FVector(0, 0, 0)) ? (std::numeric_limits<float>::max)()
                                                                      : character->GetDistanceTo(a);
                        float distB = (bPos == SDK::FVector(0, 0, 0)) ? (std::numeric_limits<float>::max)()
                                                                      : character->GetDistanceTo(b);
                        return distA < distB;
                    });

                    if (closestNucleus != nuclei.end()) {
                        auto *closestActor = *closestNucleus;
                        auto newPos = closestActor->K2_GetActorLocation();

                        if (newPos == SDK::FVector(0, 0, 0)) {
                            Logger::warning("Nucleus has invalid position. Ignoring...");
                            return;
                        }

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
            if (ImGui::Button("Teleport to closest nucleus")) {
                teleport();
            }
        }
    } // namespace TeleportNucleus
} // namespace Feats