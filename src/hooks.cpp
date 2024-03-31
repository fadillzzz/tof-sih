#include "hooks.hpp"
#include "logger/logger.hpp"
#include "minhook/include/MinHook.h"

namespace Hooks {
    std::map<std::string, std::vector<Callback>> handlers;

    typedef void(WINAPI *ProcessEvent)(SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams, void *pResult);
    ProcessEvent oProcessEvent = nullptr;

    void WINAPI myProcessEvent(SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams, void *pResult) {
        if (!pObject || !pFunction) {
            return oProcessEvent(pObject, pFunction, pParams, pResult);
        }

        const std::string functionName = pFunction->GetFullName().substr(9);

        if (handlers.find(functionName) != handlers.end()) {
            for (auto &handler : handlers[functionName]) {
                const auto result = handler(pObject, pFunction, pParams);

                if (result == STOP_EXECUTION) {
                    Logger::warning("Execution for " + functionName + " was stopped");
                    return;
                }
            }
        }

        return oProcessEvent(pObject, pFunction, pParams, pResult);
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
    }

    void registerHook(std::string functionName, Callback handler) {
        if (handlers.find(functionName) == handlers.end()) {
            handlers[functionName] = std::vector<Callback>();
        }

        handlers[functionName].push_back(handler);
    }
} // namespace Hooks