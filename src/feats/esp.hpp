namespace Feats {
    namespace Esp {
        const std::string confPrefix = "/feats/esp";
        const std::string confEnabled = confPrefix + "/enabled";
        const std::string confToggleEnabled = confPrefix + "/toggleEnabled";
        const std::string confScanDelay = confPrefix + "/scanDelay";

        void init();
        void tick();
        void menu();
        void shutdown();
    } // namespace Esp
} // namespace Feats