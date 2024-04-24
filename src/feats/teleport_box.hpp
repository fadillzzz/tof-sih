namespace Feats {
    namespace TeleportBox {
        const std::string confPrefix = "/feats/teleportBox";
        const std::string confActivate = confPrefix + "/activate";
        const std::string confIncludeRespawn = confPrefix + "/includeRespawn";
        
        void init();
        void tick();
        void menu();
    } // namespace TeleportBox
} // namespace Feats