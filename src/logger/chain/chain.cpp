#include "chain.hpp"
#include "../logger.hpp"

namespace Logger {
    namespace Chain {
        bool enabled = false;
        uint16_t minCallStackSize = 3;

        std::vector<Call> logs;
        std::map<uintptr_t, Call> threadedCallStacks;

        void enable() { enabled = true; }
        void disable() { enabled = false; }
        bool isEnabled() { return enabled; }

        void commit(Call *callStack) {
            if (std::get<uint64_t>(callStack->attributes["childCount"]) + 1 >= minCallStackSize) {
                logs.push_back(*callStack);
            }
        }

        void setMinCallStackSize(uint16_t size) { minCallStackSize = size; }

        void clearLogs() { logs.clear(); }

        void appendChild(Call *parent, Call child) {
            parent->attributes["childCount"] = std::get<uint64_t>(parent->attributes["childCount"]) + 1;

            for (auto &c : parent->children) {
                if (c.status == STARTED) {
                    appendChild(&c, child);
                    return;
                }
            }

            parent->children.push_back(child);
        }

        void startCallLog(const std::string funcName,
                          std::map<std::string, std::variant<uint64_t, std::string>> attributes) {
            if (!enabled) {
                return;
            }

            attributes["childCount"] = (uint64_t)0;

            Call call = {funcName, STARTED, std::vector<Call>{}, attributes};

            const auto callStackIt = threadedCallStacks.find(GetCurrentThreadId());

            if (callStackIt == threadedCallStacks.end()) {
                threadedCallStacks[GetCurrentThreadId()] = call;
            } else {
                const auto callStack = &(*callStackIt).second;
                if (callStack->status == COMPLETED) {
                    threadedCallStacks[GetCurrentThreadId()] = call;
                } else {
                    appendChild(callStack, call);
                }
            }
        }

        bool markCompleted(Call *callStack, const std::string &funcName,
                           std::map<std::string, std::variant<uint64_t, std::string>> *attributes) {
            for (auto &call : callStack->children) {
                if (call.status == STARTED) {
                    const auto result = markCompleted(&call, funcName, attributes);
                    if (result) {
                        return result;
                    }
                }
            }

            if (callStack->funcName == funcName && callStack->status == STARTED) {
                callStack->status = COMPLETED;

                for (const auto &attribute : *attributes) {
                    callStack->attributes[attribute.first] = attribute.second;
                }

                return true;
            }

            return false;
        }

        void endCallLog(const std::string funcName,
                        std::map<std::string, std::variant<uint64_t, std::string>> attributes) {
            if (!enabled) {
                return;
            }

            auto callStackIt = threadedCallStacks.find(GetCurrentThreadId());

            if (callStackIt != threadedCallStacks.end()) {
                auto callStack = &(*callStackIt).second;
                const auto markResult = markCompleted(callStack, funcName, &attributes);

                if (markResult) {
                    if (callStack->status == COMPLETED) {
                        commit(callStack);
                    }
                } else {
                    Logger::error("Could not find " + funcName + " in call stack.");
                    commit(callStack);
                }
            } else {
                Logger::error("Could not find call stack for thread " + std::to_string(GetCurrentThreadId()));
            }
        }

        std::vector<Call> getLogs() { return logs; }
    } // namespace Chain
} // namespace Logger
