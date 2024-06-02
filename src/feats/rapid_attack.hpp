namespace Feats {
    namespace RapidAttack {
        const std::string confPrefix = "/feats/rapidAttack";
        const std::string confEnabled = confPrefix + "/enabled";
        const std::string confToggleEnabled = confPrefix + "/toggleEnabled";
        const std::string confDisableCharAnim = confPrefix + "/disableCharAnim";

        void init();
        void tick();
        void menu();
    } // namespace RapidAttack
} // namespace Feats