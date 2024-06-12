namespace Feats {
    namespace ChestResizer {
        const std::string confPrefix = "/feats/chest_resizer";
        const std::string confEnabled = confPrefix + "/enabled";
        const std::string confScaleX = confPrefix + "/scale/x";
        const std::string confScaleY = confPrefix + "/scale/y";
        const std::string confScaleZ = confPrefix + "/scale/z";
        const std::string confTransitionX = confPrefix + "/transition/x";
        const std::string confTransitionY = confPrefix + "/transition/y";
        const std::string confTransitionZ = confPrefix + "/transition/z";

        void init();
        void tick();
        void menu();
    } // namespace ChestResizer
} // namespace Feats