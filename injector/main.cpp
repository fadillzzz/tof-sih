#include "config.hpp"
#include "inject.hpp"
#include "manual.hpp"

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

bool startLauncher(const wchar_t *launcherPath) {
    STARTUPINFO si;
    si.cb = sizeof(si);
    ZeroMemory(&si, sizeof(si));
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    SetEnvironmentVariable(L"__COMPAT_LAYER", L"RUNASINVOKER");

    const auto launcherProcessResult =
        CreateProcess(launcherPath, nullptr, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);

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

    if (!startLauncher(wideLauncherPath.c_str())) {
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

    const auto dllPath = directory + L"TOFInternal.dll";

    std::wcout << L"Injecting " + dllPath << std::endl;

    HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, qrslPid);
    bool result = false;

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
        std::cout << "Press F1 to start injection. This is preferably done at the login screen and not when the game "
                     "is loading."
                  << std::endl;
        while (true) {
            if (GetAsyncKeyState(VK_F1) & 1) {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

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