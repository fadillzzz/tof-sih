#include "resizer.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "resizer/chest.hpp"
#include "resizer/pelvis.hpp"

namespace Feats {
    namespace Resizer {
        bool isInRanking = false;

        void applyModificationToAnimInstance() {
            SDK::UQRSLAnimInstance *animInstance = nullptr;
            {
                const auto character = Globals::getCharacter();
                if (character != nullptr) {
                    animInstance = (SDK::UQRSLAnimInstance *)character->GetAnimInstance();
                }

                if (animInstance != nullptr && animInstance->IsA(SDK::UQRSLAnimInstance::StaticClass())) {
                    if (Chest::isEnabled()) {
                        Chest::applyModification(animInstance);
                    }

                    if (Pelvis::isEnabled()) {
                        Pelvis::applyModification(animInstance);
                    }
                }
            }

            {
                const auto previewActor = Globals::getObject<SDK::APreviewActor *>(SDK::APreviewActor::StaticClass());

                if (previewActor != nullptr) {
                    const auto prevChar = previewActor->PreviewCharacter;
                    if (prevChar != nullptr) {
                        const auto prevMesh = prevChar->PreviewMesh;
                        if (prevMesh != nullptr) {
                            animInstance = (SDK::UQRSLAnimInstance *)prevMesh->GetAnimInstance();

                            if (Chest::isEnabled()) {
                                Chest::applyModification(animInstance);
                            }

                            if (Pelvis::isEnabled()) {
                                Pelvis::applyModification(animInstance);
                            }
                        }
                    }
                }
            }
        }

        void init() {
            Chest::init();
            Pelvis::init();

            // Hook for Bygone Phantasm
            Hooks::registerHook(
                "HottaFramework.PreviewActor.OnBigSecretLevelSelect",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    isInRanking = false;

                    return Hooks::ExecutionFlag::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);

            // Hook for rankings & charactrer/inventory
            Hooks::registerHook(
                "BP_PreviewActor_Character.BP_PreviewActor_Character_C.ReceiveBeginPlay",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    const auto objName = pObject->GetName();

                    isInRanking = objName.contains("_Rank_");

                    return Hooks::ExecutionFlag::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);

            // Hook for actual resizing for character preview
            Hooks::registerHook(
                "Engine.AnimInstance.BlueprintInitializeAnimation",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    if (Chest::isEnabled() || Pelvis::isEnabled()) {
                        if (pObject->IsA(SDK::UQRSLAnimInstance::StaticClass())) {
                            bool isSelf = true;

                            if (isInRanking) {
                                const auto ranking =
                                    Globals::getObject<SDK::UQRSLUI_Rank *>(SDK::UQRSLUI_Rank::StaticClass());

                                const auto rotatorRatioAddr = &ranking->RotatorRatio;
                                auto selectedRowAddr = ((uint8_t *)rotatorRatioAddr - 8);

                                SDK::UQRSLUI_RankRow *selectedRow = *(SDK::UQRSLUI_RankRow **)selectedRowAddr;

                                if (selectedRow == nullptr) {
                                    selectedRow =
                                        (SDK::UQRSLUI_RankRow *)(ranking->ScrollPlayerList->Slots[0]->Content);
                                }

                                if (selectedRow != nullptr) {
                                    const auto selfRow = (SDK::UQRSLUI_RankRow *)ranking->RankRowSelf;
                                    const auto selfRank = *(int *)((uint8_t *)selfRow + 0x8B8);
                                    const auto selectedRank = *(int *)((uint8_t *)selectedRow + 0x8B8);

                                    if (selfRank != selectedRank) {
                                        isSelf = false;
                                    }
                                }
                            }

                            if (isSelf) {
                                // Intentionally creating a separate thread here, so that the execution is delayed
                                // and the game has time to initialize the preview actor, or else the values will
                                // get overwritten by the game.
                                std::thread([]() { applyModificationToAnimInstance(); }).detach();
                            }
                        }
                    }

                    return Hooks::ExecutionFlag::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);

            applyModificationToAnimInstance();
        }

        void tick() {}

        void menu() {
            Chest::menu(applyModificationToAnimInstance);
            Pelvis::menu(applyModificationToAnimInstance);
        }
    } // namespace Resizer
} // namespace Feats
