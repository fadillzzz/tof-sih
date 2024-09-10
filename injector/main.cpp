#include "config.hpp"
#include "inject.hpp"

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

HWND launcherWindow = NULL;

int main() {
    auto launcherPid = GetProcId(L"tof_launcher.exe");

    if (launcherPid != 0) {
        std::cout << "Launcher is already running." << std::endl;
        std::cout
            << "If you're running the official build of the game, please close the launcher and run the injector again."
            << std::endl;
        std::cout << "If you're running the Steam build of the game, you can ignore this message" << std::endl;
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

    launcherPid = GetProcId(L"tof_launcher.exe");
    const auto launcherProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, launcherPid);

    // Sleep for a bit to wait for the launcher to start
    Sleep(1000);

    const auto thread = InjectDll(launcherProc, directory + L"_aux.dll");
    WaitForSingleObject(thread, INFINITE);

    launcherWindow = FindWindow(L"TWINCONTROL", L"Tower of Fantasy");

    if (launcherWindow == nullptr) {
        std::cout << "Failed to find launcher window." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 1;
    }

    SendMessage(launcherWindow, WM_USER + 0x420, (uintptr_t)CreateProcessW, 0);

    std::cout << "Launcher has been started. Please start the game from the launcher." << std::endl;

    DWORD qrslPid = 0;

    while (true) {
        qrslPid = GetProcId(L"QRSL.exe");

        if (qrslPid != 0) {
            break;
        }
    }

    Config::shutdown();

    ShellExecuteW(nullptr, nullptr, L"proc.exe", std::to_wstring(qrslPid).c_str(), nullptr, SW_HIDE);

    return 0;
}