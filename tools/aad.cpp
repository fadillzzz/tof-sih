#include <Windows.h>

#include <TlHelp32.h>
#include <iostream>

#define ThreadQuerySetWin32StartAddress 9

typedef NTSTATUS(NTAPI *NtQueryInformationThread_t)(HANDLE, NTSTATUS, PVOID, ULONG, PULONG);

DWORD GetProcId(const wchar_t *procName) {
    DWORD procId = 0;
    HANDLE handleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (handleSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        while (Process32Next(handleSnapshot, &procEntry)) {
            if (_wcsicmp(procEntry.szExeFile, procName) == 0) {
                procId = procEntry.th32ProcessID;
                break;
            }
        }
    }

    CloseHandle(handleSnapshot);

    return procId;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t *moduleName) {
    uintptr_t address = 0;

    HANDLE handleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

    if (handleSnapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 moduleEntry;

        moduleEntry.dwSize = sizeof(moduleEntry);

        while (Module32Next(handleSnapshot, &moduleEntry)) {
            if (_wcsicmp(moduleEntry.szModule, moduleName) == 0) {
                address = (uintptr_t)moduleEntry.modBaseAddr;
                break;
            }
        }
    }

    CloseHandle(handleSnapshot);

    return address;
}

bool SuspendProtection(HANDLE hProcess, DWORD pid, uintptr_t protAddr) {
    THREADENTRY32 te32{};
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    te32.dwSize = sizeof(te32);
    for (Thread32First(hThreadSnap, &te32); Thread32Next(hThreadSnap, &te32);) {
        if (te32.th32OwnerProcessID == pid) {
            PVOID threadInfo;
            ULONG retLen;
            const auto ntdllHandle = GetModuleBaseAddress(pid, L"ntdll.dll");
            auto NtQueryInformationThread =
                (NtQueryInformationThread_t)GetProcAddress((HMODULE)ntdllHandle, "NtQueryInformationThread");
            if (NtQueryInformationThread == nullptr)
                return false;

            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, 0, te32.th32ThreadID);
            NTSTATUS ntqiRet =
                NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &threadInfo, sizeof(PVOID), &retLen);

            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQueryEx(hProcess, (LPCVOID)threadInfo, &mbi, sizeof(mbi))) {
                auto baseAddress = reinterpret_cast<uintptr_t>(mbi.AllocationBase);
                if (baseAddress == protAddr) {
                    std::cout << "Suspending QRSL_es.dll thread" << std::endl;
                    SuspendThread(hThread);
                    CloseHandle(hThread);
                    return true;
                }
            }
        }
    }
    CloseHandle(hThreadSnap);
    return false;
}

int main() {
    while (true) {
        const auto procId = GetProcId(L"QRSL.exe");

        const auto names = {"NtQueryAttributesFile", "NtCreateThread", "NtCreateThreadEx", "LdrInitializeThunk"};
        const auto originals = {_byteswap_uint64(0x4C8BD1B83D000000), _byteswap_uint64(0x4C8BD1B84E000000),
                                _byteswap_uint64(0x4C8BD1B8C2000000), _byteswap_uint64(0x40534883EC20488B)};

        if (procId) {
            const auto handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
            const auto QRSL_es = GetModuleBaseAddress(procId, L"QRSL_es.dll");
            const auto ntdllHandle = GetModuleBaseAddress(procId, L"ntdll.dll");

            if (ntdllHandle) {
                for (size_t i = 0; i < names.size(); i++) {
                    const auto name = *(names.begin() + i);
                    const auto original = *(originals.begin() + i);
                    const auto byteArr = reinterpret_cast<const char *>(&original);
                    const auto address = (uintptr_t)GetProcAddress((HMODULE)ntdllHandle, name);
                    if (address) {
                        size_t current;
                        ReadProcessMemory(handle, (LPCVOID)address, &current, sizeof(size_t), nullptr);

                        if (original != current) {
                            std::cout << "Restoring " << name << std::endl;
                            DWORD oldProtect;
                            VirtualProtectEx(handle, (LPVOID)address, sizeof(size_t), PAGE_EXECUTE_READWRITE,
                                             &oldProtect);
                            WriteProcessMemory(handle, (LPVOID)address, byteArr, sizeof(size_t), nullptr);
                            VirtualProtectEx(handle, (LPVOID)address, sizeof(size_t), oldProtect, nullptr);
                        }
                    }
                }
            }

            if (QRSL_es) {
                if (SuspendProtection(handle, procId, QRSL_es)) {
                    break;
                }
            }
        } else {
            std::cout << "QRSL.exe not found" << std::endl;
        }

        Sleep(2000);
    }

    return 0;
}
