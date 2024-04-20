namespace Feats {
    namespace JumpHeight {
        const std::string confPrefix = "/feats/jumpHeight";
        const std::string confHeight = confPrefix + "/height";
        const std::string confEnabled = confPrefix + "/enabled";
        const std::string confToggleEnabled = confPrefix + "/toggleEnabled";

        void init();
        void tick();
        void menu();
    } // namespace JumpHeight
} // namespace Feats