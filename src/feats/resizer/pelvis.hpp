namespace Feats {
    namespace Resizer {
        namespace Pelvis {
            const std::string confPrefix = "/feats/pelvis_resizer";
            const std::string confEnabled = confPrefix + "/enabled";
            const std::string confScaleX = confPrefix + "/scale/x";
            const std::string confScaleY = confPrefix + "/scale/y";
            const std::string confScaleZ = confPrefix + "/scale/z";

            void applyModification(SDK::UQRSLAnimInstance *animInstance);
            void init();
            void menu(std::function<void()> applyModificationToAnimInstance);
            bool isEnabled();
        } // namespace Pelvis
    } // namespace Resizer
} // namespace Feats