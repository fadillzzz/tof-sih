#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "MinHook.h"

void *createProcAddr = nullptr;
bool isSteam = false;
HANDLE gameThread = nullptr;

void rehook() {
    // For some reason immediate rehook (or not removing the hook at all) will cause
    // a crash for the Steam version of the game.
    Sleep(1);
    MH_EnableHook(createProcAddr);
}

typedef BOOL(WINAPI *createProcessW_t)(LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
                                       LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                       LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles,
                                       DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
                                       LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
createProcessW_t oCreateProcessW = nullptr;
BOOL WINAPI createProcess(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
                          LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
                          LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
                          LPPROCESS_INFORMATION lpProcessInformation) {
    const auto isGame = wcsstr(lpApplicationName, L"QRSL.exe") != nullptr;
    const auto isLauncher = wcsstr(lpApplicationName, L"x64launcher.exe") != nullptr;

    if (isGame) {
        dwCreationFlags |= CREATE_SUSPENDED;
        SetEnvironmentVariable(L"__COMPAT_LAYER", L"RUNASINVOKER");
    }

    const auto result =
        oCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
                        dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

    if (result) {
        if (isGame) {
            SetEnvironmentVariable(L"__COMPAT_LAYER", L"");
            // Wait for main injector to do its thing
            Sleep(250);
            MH_DisableHook(createProcAddr);

            if (isSteam) {
                gameThread = lpProcessInformation->hThread;
            } else {
                ResumeThread(lpProcessInformation->hThread);
            }

            // Call rehook to re-enable the hook.
            // This is important because for some reason Steam will try to use
            // thread hijacking DLL injection via a remote process and we want to make sure
            // to only resume the main game thread after that injection is finished.
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)rehook, nullptr, 0, nullptr);
        }

        if (isLauncher) {
            ResumeThread(gameThread);
        }
    }

    return result;
}

WNDPROC oWndProc = nullptr;
LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_USER + 0x420) {
        createProcAddr = (void *)wParam;
        MH_Initialize();

        MH_CreateHook(createProcAddr, createProcess, (void **)&oCreateProcessW);
        MH_EnableHook(createProcAddr);

        isSteam = GetModuleHandle(L"gameoverlayrenderer.dll") != nullptr;

        return true;
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInstDLL);
        oWndProc =
            (WNDPROC)SetWindowLongPtr(FindWindow(L"TWINCONTROL", L"Tower of Fantasy"), GWLP_WNDPROC, (LONG_PTR)wndProc);
    }

    return true;
}
