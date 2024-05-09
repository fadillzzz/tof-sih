
#include <Windows.h>
#include <iostream>
#include <winternl.h>

#include "Minhook.h"
#include <map>
#include <string>

typedef FARPROC (*GetProcAddress_t)(HMODULE hModule, LPCSTR lpProcName);
GetProcAddress_t oGetProcAddress = nullptr;

#define ThreadQuerySetWin32StartAddress 9

struct funcInfo {
    HMODULE hModule;
    LPCSTR lpProcName;
    uintptr_t address;
    size_t original;
};

std::map<std::wstring, funcInfo> potentiallyHookedFunctions;

HMODULE WINAPI getThreadBase(HANDLE hThread) {
    NTSTATUS ntStatus;
    HANDLE hDupHandle;
    PVOID threadInfo;

    HANDLE hCurrentProcess = GetCurrentProcess();
    if (!DuplicateHandle(hCurrentProcess, hThread, hCurrentProcess, &hDupHandle, THREAD_QUERY_INFORMATION, FALSE, 0)) {
        SetLastError(ERROR_ACCESS_DENIED);

        return 0;
    }

    ntStatus = NtQueryInformationThread(hDupHandle, (THREADINFOCLASS)ThreadQuerySetWin32StartAddress, &threadInfo,
                                        sizeof(PVOID), NULL);
    CloseHandle(hDupHandle);

    if (!NT_SUCCESS(ntStatus)) {
        return 0;
    }

    MEMORY_BASIC_INFORMATION mbi;

    if (VirtualQuery(threadInfo, &mbi, sizeof(mbi))) {
        return (HMODULE)mbi.AllocationBase;
    }

    return 0;
}

FARPROC GetProcAddressHook(HMODULE hModule, LPCSTR lpProcName) {
    const auto threadBase = getThreadBase(GetCurrentThread());
    const auto QRSL_es = GetModuleHandle(L"QRSL_es.dll");

    if (threadBase == QRSL_es) {
        wchar_t fileName[MAX_PATH];
        GetModuleFileName(hModule, fileName, MAX_PATH);
        wchar_t wideProcName[256];
        MultiByteToWideChar(CP_UTF8, 0, lpProcName, -1, wideProcName, 256);
        const auto fullName = std::wstring(fileName) + L"!" + wideProcName;

        if (potentiallyHookedFunctions.find(fullName) == potentiallyHookedFunctions.end()) {
            std::wcout << L"New function being retrieved: " << fullName << std::endl;
            funcInfo info;
            info.hModule = hModule;
            info.lpProcName = lpProcName;
            info.address = (uintptr_t)oGetProcAddress(hModule, lpProcName);
            info.original = *(size_t *)info.address;
            potentiallyHookedFunctions[std::wstring(wideProcName)] = info;
        } else {
            std::wcout << L"Seen function: " << fullName << std::endl;
        }
    }

    return oGetProcAddress(hModule, lpProcName);
}

void mainThread() {
    AllocConsole();
    freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
    MH_Initialize();
    MH_CreateHook(GetProcAddress, (LPVOID)GetProcAddressHook, (LPVOID *)&oGetProcAddress);
    MH_EnableHook(GetProcAddress);
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        HANDLE thread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)mainThread, nullptr, 0, nullptr);
        if (thread) {
            CloseHandle(thread);
        }
        DisableThreadLibraryCalls(hInstDLL);
    }
    return TRUE;
}
