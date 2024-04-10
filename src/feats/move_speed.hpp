namespace Feats {
    namespace MoveSpeed {
        const std::string confPrefix = "/feats/moveSpeed";
        const std::string confSpeed = confPrefix + "/speed";
        const std::string confEnabled = confPrefix + "/enabled";
        const std::string confToggleEnabled = confPrefix + "/toggleEnabled";

        void init();
        void tick();
        void menu();
    } // namespace MoveSpeed
} // namespace Feats