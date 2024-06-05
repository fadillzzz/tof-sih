namespace Feats {
    namespace AbilityFailure {
        const std::string confPrefix = "/feats/abilityFailure";
        const std::string confEnabled = confPrefix + "/enabled";

        void init();
        void tick();
        void menu();

        void enableAndLock();
        void unlock();
    } // namespace AbilityFailure
} // namespace Feats