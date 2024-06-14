#include "pelvis.hpp"

namespace Feats {
    namespace Resizer {
        namespace Pelvis {
            Config::field<bool> enabled;

            Config::field<double> scaleX;
            Config::field<double> scaleY;
            Config::field<double> scaleZ;

            double min = 0.0l;
            double max = 10.0l;

            void applyModification(SDK::UQRSLAnimInstance *animInstance) {
                animInstance->BonesData.PelvisScale = SDK::FVector(*scaleX, *scaleY, *scaleZ);
            }

            void init() {
                enabled = Config::get<bool>(confEnabled, false);

                scaleX = Config::get<double>(confScaleX, 1.0l);
                scaleY = Config::get<double>(confScaleY, 1.0l);
                scaleZ = Config::get<double>(confScaleZ, 1.0l);
            }

            void menu(std::function<void()> applyModificationToAnimInstance) {
                ImGui::Checkbox("Pelvis Resizer", &enabled);
                ImGui::Indent();

                ImGui::Text("Scale");
                ImGui::Indent();
                {
                    const auto labels = {"X", "Y", "Z"};
                    const auto values = {&scaleX, &scaleY, &scaleZ};
                    for (auto i = 0; i < 3; i++) {
                        const auto label = std::string(labels.begin()[i]) + "##PelvisScale";
                        if (ImGui::SliderScalar(label.c_str(), ImGuiDataType_Double, values.begin()[i], &min, &max)) {
                            applyModificationToAnimInstance();
                        }
                    }
                }
                ImGui::Unindent();

                ImGui::Unindent();
            }

            bool isEnabled() { return *enabled; }
        } // namespace Pelvis
    } // namespace Resizer
} // namespace Feats