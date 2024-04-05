#include "chain.hpp"
#include "../logger.hpp"

namespace Logger {
    namespace Chain {
        bool enabled = false;
        uint16_t minCallStackSize = 3;

        std::vector<std::vector<Call>> logs;
        std::map<uintptr_t, std::vector<Call>> threadedCallStacks;

        void enable() { enabled = true; }
        void disable() { enabled = false; }
        bool isEnabled() { return enabled; }

        void commit(std::vector<Call> *callStack) {
            if (callStack->size() >= minCallStackSize) {
                logs.push_back(*callStack);
            }

            callStack->clear();
        }

        void setMinCallStackSize(uint16_t size) { minCallStackSize = size; }

        void clearLogs() { logs.clear(); }

        void startCallLog(const std::string funcName, std::map<std::string, std::string> attributes) {
            if (!enabled) {
                return;
            }

            Call call = {funcName, STARTED, attributes};

            threadedCallStacks[GetCurrentThreadId()].push_back(call);
        }

        void endCallLog(const std::string funcName, std::map<std::string, std::string> attributes) {
            if (!enabled) {
                return;
            }

            auto callStack = &threadedCallStacks[GetCurrentThreadId()];

            auto i = callStack->rbegin();
            for (; i != callStack->rend(); ++i) {
                if (i->funcName == funcName && i->status == STARTED) {
                    i->status = COMPLETED;
                    break;
                }
            }

            if (i != callStack->rend()) {
                for (const auto &attribute : attributes) {
                    i->attributes[attribute.first] = attribute.second;
                }
            }

            if (i == callStack->rend()) {
                Logger::error("Could not find " + funcName + " in call stack.");
                commit(callStack);
            } else if (++i == callStack->rend()) {
                commit(callStack);
            }
        }

        std::vector<std::vector<Call>> getLogs() { return logs; }
    } // namespace Chain
} // namespace Logger
