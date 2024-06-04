#include "config.hpp"
#include "inject.hpp"
#include "manual.hpp"

#define ThreadQuerySetWin32StartAddress 9

typedef NTSTATUS(NTAPI *NtQueryInformationThread_t)(HANDLE, NTSTATUS, PVOID, ULONG, PULONG);

typedef void (*setDirectory)(std::wstring directory);
typedef int (*init)(HINSTANCE hInstDLL);

PWSTR askForLauncherPath() {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    PWSTR pszFilePath;

    if (SUCCEEDED(hr)) {
        IFileOpenDialog *pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog,
                              reinterpret_cast<void **>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            const COMDLG_FILTERSPEC filter[] = {
                {L"TOF Launcher", L"tof_launcher.exe"},
            };
            pFileOpen->SetFileTypes(ARRAYSIZE(filter), filter);
            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr)) {
                IShellItem *pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {

                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr)) {
                        return pszFilePath;
                    }

                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }

    return pszFilePath;
}

bool startLauncher(wchar_t *launcherPath) {
    STARTUPINFO si;
    si.cb = sizeof(si);
    ZeroMemory(&si, sizeof(si));
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    SetEnvironmentVariable(L"__COMPAT_LAYER", L"RUNASINVOKER");

    const auto launcherProcessResult =
        CreateProcess(nullptr, launcherPath, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);

    if (!launcherProcessResult) {
        std::cout << "Failed to start the launcher. Exiting..." << std::endl;
        std::cout << "Error: " << GetLastError() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return false;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

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

uintptr_t GetModuleBaseAddress(HANDLE handleSnapshot, const wchar_t *moduleName) {
    uintptr_t address = 0;

    if (handleSnapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 moduleEntry;

        moduleEntry.dwSize = sizeof(moduleEntry);

        if (!Module32First(handleSnapshot, &moduleEntry)) {
            return address;
        }

        do {
            if (_wcsicmp(moduleEntry.szModule, moduleName) == 0) {
                address = (uintptr_t)moduleEntry.modBaseAddr;
                break;
            }
        } while (Module32Next(handleSnapshot, &moduleEntry));
    }

    return address;
}

bool SuspendProtection(HANDLE hProcess, DWORD pid, uintptr_t protAddr, HANDLE ntdllHandle) {
    THREADENTRY32 te32{};
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    te32.dwSize = sizeof(te32);
    for (Thread32First(hThreadSnap, &te32); Thread32Next(hThreadSnap, &te32);) {
        if (te32.th32OwnerProcessID == pid) {
            PVOID threadInfo;
            ULONG retLen;
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
                    std::cout << "Suspending protection thread" << std::endl;
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

void undoProtection(HANDLE handle) {
    std::cout << "Attempting to undo protection..." << std::endl;

    const auto procId = GetProcessId(handle);
    const auto names = {"NtQueryAttributesFile", "NtCreateThread", "NtCreateThreadEx", "LdrInitializeThunk"};
    const auto originals = {_byteswap_uint64(0x4C8BD1B83D000000), _byteswap_uint64(0x4C8BD1B84E000000),
                            _byteswap_uint64(0x4C8BD1B8C2000000), _byteswap_uint64(0x40534883EC20488B)};

    uintptr_t QRSL_es = 0;
    uintptr_t ntdllHandle = 0;
    uint8_t restored = 0;

    do {
        const auto moduleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
        QRSL_es = GetModuleBaseAddress(moduleSnapshot, L"QRSL_es.dll");
        ntdllHandle = GetModuleBaseAddress(moduleSnapshot, L"ntdll.dll");
        CloseHandle(moduleSnapshot);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    } while (!QRSL_es || !ntdllHandle);

    std::cout << std::hex;
    std::cout << "QRSL_es.dll: " << QRSL_es << std::endl;
    std::cout << "ntdll.dll: " << ntdllHandle << std::endl;

    std::cout << "Waiting for a few seconds before restoring hooked functions..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(10));

    while (restored < names.size()) {
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
                    VirtualProtectEx(handle, (LPVOID)address, sizeof(size_t), PAGE_EXECUTE_READWRITE, &oldProtect);
                    WriteProcessMemory(handle, (LPVOID)address, byteArr, sizeof(size_t), nullptr);
                    VirtualProtectEx(handle, (LPVOID)address, sizeof(size_t), oldProtect, nullptr);
                    restored++;
                }
            }
        }
    }

    SuspendProtection(handle, procId, QRSL_es, (HANDLE)ntdllHandle);
}

int main() {
    const auto launcherPid = GetProcId(L"tof_launcher.exe");

    if (launcherPid != 0) {
        std::cout << "Launcher is already running." << std::endl;
        std::cout << "If it wasn't started by this injector. Please close it and launch the injector again."
                  << std::endl;
    }

    const uint16_t pathSize = 2048;
    wchar_t path[pathSize];
    GetModuleFileName(nullptr, (LPWSTR)path, sizeof(path));
    std::wstring directory = std::wstring(path);
    directory = directory.substr(0, directory.find_last_of(L"\\") + 1);

    Config::setDirectory(directory);
    Config::init();

    auto launcherPath = Config::get<std::string>("/launcherPath", "");

    if (launcherPath->empty()) {
        std::wcout << L"Launcher path not found. Please select the launcher path." << std::endl;

        const auto path = std::wstring(askForLauncherPath());

        if (!path.empty()) {
            char multiBytePath[2048];
            const auto convertRes =
                WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, multiBytePath, 2048, nullptr, nullptr);
            launcherPath = std::string(multiBytePath);
        } else {
            std::wcout << L"Launcher path not given. Exiting..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));
            return 1;
        }
    }

    auto wideLauncherPath = std::wstring(launcherPath->begin(), launcherPath->end());

    if (!startLauncher(wideLauncherPath.data())) {
        return 1;
    }

    auto configuredInjectionMethod = Config::get<std::string>("/injectionMethod", "");
    std::string injectionMethod = "loadLibrary";

    if (!configuredInjectionMethod->empty()) {
        injectionMethod = std::string(configuredInjectionMethod->begin(), configuredInjectionMethod->end());
    } else {
        configuredInjectionMethod = injectionMethod;
    }

    std::cout << "Injection method: " << injectionMethod << std::endl;

    std::cout << "Launcher has been started. Please start the game from the launcher." << std::endl;

    DWORD qrslPid = 0;

    while (true) {
        qrslPid = GetProcId(L"QRSL.exe");

        if (qrslPid != 0) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, qrslPid);
    bool result = false;

    undoProtection(proc);

    const auto dllPath = directory + L"TOFInternal.dll";

    std::wcout << L"Injecting " + dllPath << std::endl;

    if (injectionMethod == "manual") {
        std::ifstream dllFile(dllPath, std::ios::binary | std::ios::ate);
        auto dllSize = dllFile.tellg();
        BYTE *pSrcData = new BYTE[(UINT_PTR)dllSize];
        dllFile.seekg(0, std::ios::beg);
        dllFile.read((char *)(pSrcData), dllSize);
        dllFile.close();

        result =
            ManualMapDll<const wchar_t *>(proc, pSrcData, dllSize, "preMain", directory.c_str(), directory.size() * 2);

        delete[] pSrcData;
    } else if (injectionMethod == "loadLibrary") {
        result = InjectDll(proc, dllPath.c_str());
    } else {
        std::cout << "Invalid injection method." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 1;
    }

    if (result) {
        std::cout << "Injected successfully." << std::endl;
    } else {
        std::cout << "Failed to inject." << std::endl;
    }

    CloseHandle(proc);

    Config::shutdown();

    std::this_thread::sleep_for(std::chrono::seconds(4));

    return 0;
}