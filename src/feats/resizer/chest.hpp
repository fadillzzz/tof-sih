namespace Feats {
    namespace Resizer {
        namespace Chest {
            const std::string confPrefix = "/feats/chest_resizer";
            const std::string confEnabled = confPrefix + "/enabled";
            const std::string confScaleX = confPrefix + "/scale/x";
            const std::string confScaleY = confPrefix + "/scale/y";
            const std::string confScaleZ = confPrefix + "/scale/z";
            const std::string confTransitionX = confPrefix + "/transition/x";
            const std::string confTransitionY = confPrefix + "/transition/y";
            const std::string confTransitionZ = confPrefix + "/transition/z";

            void applyModification(SDK::UQRSLAnimInstance *animInstance);
            void init();
            void menu(std::function<void()> applyModificationToAnimInstance);
            bool isEnabled();
        } // namespace Chest
    } // namespace Resizer
} // namespace Feats