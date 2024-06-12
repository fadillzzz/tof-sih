namespace Feats {
    namespace NoTransparency {
        const std::string confPrefix = "/feats/no_transparency";
        const std::string confEnabled = confPrefix + "/enabled";
        const std::string confToggleEnabled = confPrefix + "/toggleEnabled";

        void init();
        void tick();
        void menu();
    } // namespace NoTransparency
} // namespace Feats