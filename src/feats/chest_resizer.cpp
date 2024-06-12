#include "chest_resizer.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"

namespace Feats {
    namespace ChestResizer {
        Config::field<bool> enabled;

        Config::field<double> scaleX;
        Config::field<double> scaleY;
        Config::field<double> scaleZ;

        Config::field<double> transitionX;
        Config::field<double> transitionY;
        Config::field<double> transitionZ;

        bool isInRanking = false;

        double min = 0.0l;
        double max = 10.0l;

        void applyModificationToAnimInstance() {
            if (*enabled) {
                SDK::UQRSLAnimInstance *animInstance = nullptr;
                {
                    const auto character = Globals::getCharacter();
                    if (character != nullptr) {
                        animInstance = (SDK::UQRSLAnimInstance *)character->GetAnimInstance();
                    }

                    if (animInstance != nullptr) {
                        animInstance->HeadBonesData.ChestScale = SDK::FVector(*scaleX, *scaleY, *scaleZ);
                        animInstance->HeadBonesData.ChestTransition =
                            SDK::FVector(*transitionX, *transitionY, *transitionZ);
                    }
                }

                {
                    const auto previewActor =
                        Globals::getObject<SDK::APreviewActor *>(SDK::APreviewActor::StaticClass());

                    if (previewActor != nullptr) {
                        const auto prevChar = previewActor->PreviewCharacter;
                        if (prevChar != nullptr) {
                            const auto prevMesh = prevChar->PreviewMesh;
                            if (prevMesh != nullptr) {
                                animInstance = (SDK::UQRSLAnimInstance *)prevMesh->GetAnimInstance();

                                animInstance->HeadBonesData.ChestScale = SDK::FVector(*scaleX, *scaleY, *scaleZ);
                                animInstance->HeadBonesData.ChestTransition =
                                    SDK::FVector(*transitionX, *transitionY, *transitionZ);
                            }
                        }
                    }
                }
            }
        }

        void init() {
            enabled = Config::get<bool>(confEnabled, false);

            scaleX = Config::get<double>(confScaleX, 1.0l);
            scaleY = Config::get<double>(confScaleY, 1.0l);
            scaleZ = Config::get<double>(confScaleZ, 1.0l);

            transitionX = Config::get<double>(confTransitionX, 1.0l);
            transitionY = Config::get<double>(confTransitionY, 1.0l);
            transitionZ = Config::get<double>(confTransitionZ, 1.0l);

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

            // Hook for actual chest resizing for character preview
            Hooks::registerHook(
                "Engine.AnimInstance.BlueprintInitializeAnimation",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    if (*enabled) {
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
                                // and the game has time to initialize the preview actor, or else the values will get
                                // overwritten by the game.
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
            ImGui::Checkbox("Chest Resizer", &enabled);
            ImGui::Indent();

            ImGui::Text("Scale");
            ImGui::Indent();
            {
                const auto labels = {"X", "Y", "Z"};
                const auto values = {&scaleX, &scaleY, &scaleZ};
                for (auto i = 0; i < 3; i++) {
                    const auto label = std::string(labels.begin()[i]) + "##ChestScale";
                    if (ImGui::SliderScalar(label.c_str(), ImGuiDataType_Double, values.begin()[i], &min, &max)) {
                        applyModificationToAnimInstance();
                    }
                }
            }
            ImGui::Unindent();

            ImGui::Text("Spacing");
            ImGui::Indent();
            {
                const auto labels = {"X", "Y", "Z"};
                const auto values = {&transitionX, &transitionY, &transitionZ};
                for (auto i = 0; i < 3; i++) {
                    const auto label = std::string(labels.begin()[i]) + "##ChestTransition";
                    if (ImGui::SliderScalar(label.c_str(), ImGuiDataType_Double, values.begin()[i], &min, &max)) {
                        applyModificationToAnimInstance();
                    }
                }
            }
            ImGui::Unindent();

            ImGui::Unindent();
        }
    } // namespace ChestResizer
} // namespace Feats