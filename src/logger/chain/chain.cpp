#include "chain.hpp"
#include "../logger.hpp"
#include <mutex>

namespace Logger {
    namespace Chain {
        bool enabled = false;
        uint16_t minCallStackSize = 3;

        std::mutex mutex;

        std::vector<std::vector<Call>> logs;
        std::vector<Call> callStack;

        void enable() { enabled = true; }
        void disable() { enabled = false; }
        bool isEnabled() { return enabled; }

        void commit() {
            if (callStack.size() >= minCallStackSize) {
                logs.push_back(callStack);
            }

            callStack.clear();
        }

        void setMinCallStackSize(uint16_t size) { minCallStackSize = size; }

        void clearLogs() { logs.clear(); }

        void startCallLog(const std::string funcName, std::map<std::string, std::string> attributes) {
            if (!enabled) {
                return;
            }

            std::lock_guard<std::mutex> lock(mutex);

            Call call = {funcName, STARTED, attributes};

            callStack.push_back(call);
        }

        void endCallLog(const std::string funcName, std::map<std::string, std::string> attributes) {
            if (!enabled) {
                return;
            }

            std::lock_guard<std::mutex> lock(mutex);

            auto i = callStack.rbegin();
            for (; i != callStack.rend(); ++i) {
                if (i->funcName == funcName && i->status == STARTED) {
                    i->status = COMPLETED;
                    break;
                }
            }

            if (i != callStack.rend()) {
                for (const auto &attribute : attributes) {
                    i->attributes[attribute.first] = attribute.second;
                }
            }

            if (i == callStack.rend()) {
                Logger::error("Could not find " + funcName + " in call stack.");
                commit();
            } else if (++i == callStack.rend()) {
                commit();
            }
        }

        std::vector<std::vector<Call>> getLogs() { return logs; }
    } // namespace Chain
} // namespace Logger
