namespace Feats {
    namespace EarlyInit {
        const std::string confPrefix = "/feats/earlyInit";
        const std::string confEnabled = confPrefix + "/enabled";

        void init();
        void tick();
        void menu();
    } // namespace EarlyInit
} // namespace Feats