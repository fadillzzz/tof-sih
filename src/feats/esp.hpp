namespace Feats {
    namespace Esp {
        const std::string confPrefix = "/feats/esp";
        const std::string confEnabled = confPrefix + "/enabled";
        const std::string confToggleEnabled = confPrefix + "/toggleEnabled";
        const std::string confScanDelay = confPrefix + "/scanDelay";
        const std::string confdisableDistanceLimit = confPrefix + "/disableDistanceLimit";
        const std::string confMaxDistance = confPrefix + "/maxDistance";

        // visual drawing
        const std::string confDrawBox = confPrefix + "/drawBox";
        const std::string confDrawTracer = confPrefix + "/drawTracer";
        const std::string confDrawName = confPrefix + "/drawName";
        const std::string confDrawDistance = confPrefix + "/drawDistance";

        // appearance
        const std::string confFontSize = confPrefix + "/fontSize";
        const std::string confOutlineWidth = confPrefix + "/outlineWidth";
        const std::string confTracerWidth = confPrefix + "/tracerWidth";
        const std::string confTransparency = confPrefix + "/transparency";

        // color
        const std::string confBoxColor = confPrefix + "/boxColor";
        const std::string confTracerColor = confPrefix + "/tracerColor";
        const std::string confTextColor = confPrefix + "/textColor";

        // filter
        const std::string confShowSupplyBox = confPrefix + "/showSupplyBox";
        const std::string confShowNucleus = confPrefix + "/showNucleus";
        const std::string confShowKerosenia = confPrefix + "/showKerosenia";
        const std::string confShowPerspective = confPrefix + "/showPerspective";
        const std::string confShowWatcher = confPrefix + "/showWatcher";
        const std::string confShowShroom = confPrefix + "/showShroom";
        const std::string confShowDandelion = confPrefix + "/showDandelion";
        const std::string confShowChowchow = confPrefix + "/showChowchow";
        const std::string confShowSponge = confPrefix + "/showSponge";
        const std::string confShowParticleFish = confPrefix + "/showParticleFish";
        const std::string confShowFishBaiter = confPrefix + "/showFishBaiter";

        void init();
        void tick();
        void menu();
        void shutdown();
    } // namespace Esp
} // namespace Feats