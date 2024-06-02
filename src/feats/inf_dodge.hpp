namespace Feats {
    namespace InfDodge {
        const std::string confPrefix = "/feats/infiniteDodge";
        const std::string confEnabled = confPrefix + "/enabled";
        const std::string confToggleEnabled = confPrefix + "/toggleEnabled";

        void init();
        void tick();
        void menu();
    } // namespace InfDodge
} // namespace Feats