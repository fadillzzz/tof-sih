#include "chest.hpp"

namespace Feats {
    namespace Resizer {
        namespace Chest {
            Config::field<bool> enabled;

            Config::field<double> scaleX;
            Config::field<double> scaleY;
            Config::field<double> scaleZ;

            Config::field<double> transitionX;
            Config::field<double> transitionY;
            Config::field<double> transitionZ;

            double min = 0.0l;
            double max = 10.0l;

            void applyModification(SDK::UQRSLAnimInstance *animInstance) {
                animInstance->HeadBonesData.ChestScale = SDK::FVector(*scaleX, *scaleY, *scaleZ);
                animInstance->HeadBonesData.ChestTransition = SDK::FVector(*transitionX, *transitionY, *transitionZ);
            }

            void init() {
                enabled = Config::get<bool>(confEnabled, false);

                scaleX = Config::get<double>(confScaleX, 1.0l);
                scaleY = Config::get<double>(confScaleY, 1.0l);
                scaleZ = Config::get<double>(confScaleZ, 1.0l);

                transitionX = Config::get<double>(confTransitionX, 1.0l);
                transitionY = Config::get<double>(confTransitionY, 1.0l);
                transitionZ = Config::get<double>(confTransitionZ, 1.0l);
            }

            void menu(std::function<void()> applyModificationToAnimInstance) {
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

            bool isEnabled() { return *enabled; }
        } // namespace Chest
    } // namespace Resizer
} // namespace Feats