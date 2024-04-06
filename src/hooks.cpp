#include "hooks.hpp"
#include "logger/logger.hpp"
#include "minhook/include/MinHook.h"

#define RETURN_IF_STOPPING(result, message)                                                                            \
    if (result == STOP_EXECUTION) {                                                                                    \
        Logger::warning(message);                                                                                      \
        return;                                                                                                        \
    }

namespace Hooks {
    std::map<Type, std::map<std::string, std::vector<Callback>>> handlers = {
        {Type::PRE, std::map<std::string, std::vector<Callback>>()},
        {Type::POST, std::map<std::string, std::vector<Callback>>()},
    };

    typedef void(WINAPI *ProcessEvent)(SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams, void *pResult);
    ProcessEvent oProcessEvent = nullptr;

    ExecutionFlag runHooks(Type type, const std::string &functionName, SDK::UObject *pObject, SDK::UFunction *pFunction,
                           void *pParams) {
        if (handlers[type].find(functionName) != handlers[type].end()) {
            for (auto &handler : handlers[type][functionName]) {
                const auto result = handler(pObject, pFunction, pParams);

                if (result == STOP_EXECUTION) {
                    return STOP_EXECUTION;
                }
            }
        }

        return CONTINUE_EXECUTION;
    }

    void WINAPI myProcessEvent(SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams, void *pResult) {
        if (!pObject || !pFunction) {
            return oProcessEvent(pObject, pFunction, pParams, pResult);
        }

        const std::string functionName = pFunction->GetFullName().substr(9);

        const auto preResult = runHooks(Type::PRE, functionName, pObject, pFunction, pParams);
        RETURN_IF_STOPPING(preResult, "Pre-hooks: Execution for " + functionName + " was stopped");

        const auto preWildResult = runHooks(Type::PRE, "*", pObject, pFunction, pParams);
        RETURN_IF_STOPPING(preWildResult, "Pre-hooks (Wildcard): Execution for " + functionName + " was stopped");

        oProcessEvent(pObject, pFunction, pParams, pResult);

        const auto postResult = runHooks(Type::POST, functionName, pObject, pFunction, pParams);
        RETURN_IF_STOPPING(postResult, "Post-hooks: Execution for " + functionName + " was stopped");

        const auto postWildResult = runHooks(Type::POST, "*", pObject, pFunction, pParams);
        RETURN_IF_STOPPING(postWildResult, "Post-hooks (Wildcard): Execution for " + functionName + " was stopped");
    }

    void init() {
        const auto base = SDK::InSDKUtils::GetImageBase();

        // Menu::init() implicitly calls MH_Initialize() so we can just go ahead and hook the functions
        MH_CreateHook((LPVOID)(base + SDK::Offsets::ProcessEvent), myProcessEvent, (LPVOID *)&oProcessEvent);
        MH_EnableHook((LPVOID)(base + SDK::Offsets::ProcessEvent));
    }

    void shutdown() {
        const auto base = SDK::InSDKUtils::GetImageBase();

        MH_DisableHook((LPVOID)(base + SDK::Offsets::ProcessEvent));
        MH_RemoveHook((LPVOID)(base + SDK::Offsets::ProcessEvent));
        MH_Uninitialize();
    }

    void registerHook(std::string functionName, Callback handler, Type type) {
        if (handlers[type].find(functionName) == handlers[type].end()) {
            handlers[type][functionName] = std::vector<Callback>();
        }

        handlers[type][functionName].push_back(handler);
    }
} // namespace Hooks