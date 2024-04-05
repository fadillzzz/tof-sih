#include <variant>

namespace Logger {
    namespace Chain {
        enum Status { STARTED, COMPLETED };

        struct Call {
            std::string funcName;
            Status status;
            std::vector<Call> children;
            std::map<std::string, std::variant<uint64_t, std::string>> attributes;
        };

        void enable();
        void disable();
        bool isEnabled();
        void startCallLog(const std::string funcName,
                          std::map<std::string, std::variant<uint64_t, std::string>> attributes = {});
        void endCallLog(const std::string funcName,
                        std::map<std::string, std::variant<uint64_t, std::string>> attributes = {});
        void setMinCallStackSize(uint16_t size);
        void clearLogs();
        std::vector<Call> getLogs();
    } // namespace Chain
} // namespace Logger
