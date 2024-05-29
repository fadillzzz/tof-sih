namespace Feats {
    namespace Quest {
        const std::string confPrefix = "/feats/quest";
        const std::string confActivateMain = confPrefix + "/activateMain";
        const std::string confActivateDaily = confPrefix + "/activateDaily";
        const std::string confActivateWeekly = confPrefix + "/activateWeekly";
        const std::string confActivateAll = confPrefix + "/activateAll";
        const std::string confActivateAllExceptMainEnabled = confPrefix + "/activateAllExceptMainEnabled";

        void init();
        void tick();
        void menu();
    } // namespace Quest
} // namespace Feats