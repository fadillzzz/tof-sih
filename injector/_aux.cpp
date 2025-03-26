#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>
#include <vector>

#include "../src/memory_manager.hpp"
#include "MinHook.h"
#include "inject.hpp"

void **createProcAddr = nullptr;
void **createFileAddr = nullptr;
void **deviceIoCtrlAddr = nullptr;
HANDLE myHtHandle = (void *)0xBAADC0DE;

typedef BOOL(WINAPI *deviceIoCtrl_t)(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize,
                                     LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned,
                                     LPOVERLAPPED lpOverlapped);
deviceIoCtrl_t oDeviceIoCtrl = nullptr;
BOOL WINAPI deviceIoCtrl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize,
                         LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped) {
    if (hDevice == myHtHandle) {
        if (dwIoControlCode == 0x222004) {
            *(int *)lpOutBuffer = 0x73B6A;
            return true;
        }

        if (dwIoControlCode == 0x222008) {
            if (lpOutBuffer != nullptr) {
                *(int *)lpOutBuffer = 0;
            }
            return true;
        }

        std::wstring msg = L"Unknown deviceIoCtrlCode: " + std::to_wstring(dwIoControlCode);

        MessageBox(nullptr, msg.data(), L"Error", MB_ICONERROR);

        return true;
    }

    return oDeviceIoCtrl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize,
                         lpBytesReturned, lpOverlapped);
}

typedef HANDLE(WINAPI *createFileA_t)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                                      LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                                      DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
createFileA_t oCreateFileA = nullptr;
HANDLE WINAPI createFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                         LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                         DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    const auto fileName = std::string(lpFileName);

    if (fileName.contains("AntiCheat")) {
        return myHtHandle;
    }

    return oCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
                        dwFlagsAndAttributes, hTemplateFile);
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
    const auto isGame = lpApplicationName != nullptr ? wcsstr(lpApplicationName, L"QRSL.exe") != nullptr : false;

    const auto result =
        oCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
                        dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

    if (isGame) {
        const uint16_t pathSize = 4096;
        wchar_t dir[pathSize];
        GetEnvironmentVariable(L"_____DIR", dir, pathSize);
        std::wstring path = dir;
        path += L"\\TOFInternal.dll";
        InjectDll(lpProcessInformation->hProcess, path);
    }

    return result;
}

void start() {
    MH_Initialize();

    while (createProcAddr == nullptr) {
        const auto mod = GetModuleHandle(L"kernelbase.dll");
        if (mod != nullptr) {
            createProcAddr = (void **)GetProcAddress(mod, "CreateProcessW");
            createFileAddr = (void **)GetProcAddress(mod, "CreateFileA");
            deviceIoCtrlAddr = (void **)GetProcAddress(mod, "DeviceIoControl");
            break;
        }
        Sleep(100);
    }

    MH_CreateHook(createProcAddr, (void **)createProcess, (void **)&oCreateProcessW);
    MH_EnableHook(createProcAddr);

    MH_CreateHook(createFileAddr, (void **)createFile, (void **)&oCreateFileA);
    MH_EnableHook(createFileAddr);

    MH_CreateHook(deviceIoCtrlAddr, (void **)deviceIoCtrl, (void **)&oDeviceIoCtrl);
    MH_EnableHook(deviceIoCtrlAddr);

    MemoryManager memory;
    std::vector<void *> result =
        memory.PatternScan("48 89 4C 24 08 48 83 EC 48 48 8B 44 24 50 48 89 44 24 28 C7 44 24 20 00 00 00 00");

    if (!result.empty()) {
        const auto addr = result[0];
        DWORD oldProtect;
        VirtualProtect(addr, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
        *(uint8_t *)addr = 0xC3;
        VirtualProtect(addr, 1, oldProtect, &oldProtect);
    }
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInstDLL);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)start, nullptr, 0, nullptr);
    }

    return true;
}
