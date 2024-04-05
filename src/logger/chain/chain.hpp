namespace Logger {
    namespace Chain {
        enum Status { STARTED, COMPLETED };

        struct Call {
            std::string funcName;
            Status status;
        };

        void enable();
        void disable();
        bool isEnabled();
        void startCallLog(const std::string funcName);
        void endCallLog(const std::string funcName);
        void setMinCallStackSize(uint16_t size);
        void clearLogs();
        std::vector<std::vector<Call>> getLogs();
    } // namespace Chain
} // namespace Logger
