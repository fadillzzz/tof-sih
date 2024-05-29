namespace Feats {
    namespace NoClip {
        const std::string confPrefix = "/feats/noClip";
        const std::string confEnabled = confPrefix + "/enabled";
        const std::string confToggleEnabled = confPrefix + "/toggleEnabled";

        void init();
        void tick();
        void menu();
    } // namespace NoClip
} // namespace Feats