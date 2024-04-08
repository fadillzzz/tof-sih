#define WIN32_LEAN_AND_MEAN

#include "ManualMap/MMap.h"
#include "Process/Process.h"
#include "config.hpp"
#include <shellapi.h>

typedef void (*setDirectory)(std::wstring directory);

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

bool startLauncher(std::string *launcherPath) {
    STARTUPINFO si;
    si.cb = sizeof(si);
    ZeroMemory(&si, sizeof(si));
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    SetEnvironmentVariable(L"__COMPAT_LAYER", L"RUNASINVOKER");

    wchar_t *wideLauncherPath = (wchar_t *)std::wstring(launcherPath->begin(), launcherPath->end()).c_str();
    const auto launcherProcessResult =
        CreateProcess(wideLauncherPath, wideLauncherPath, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);

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

int main() {
    blackbone::Process launcherChecker;

    const auto checkResult = launcherChecker.Attach(L"tof_launcher.exe");

    if (NT_SUCCESS(checkResult)) {
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

    auto launcherPath = Config::get<nlohmann::json::string_t>("/launcherPath", "");

    if (launcherPath->empty()) {
        std::wcout << L"Launcher path not found. Please select the launcher path." << std::endl;

        const auto path = std::wstring(askForLauncherPath());

        if (!path.empty()) {
            const auto multiBytePath = _bstr_t(path.c_str());
            Config::config["launcherPath"] = multiBytePath;
            Config::save();
            launcherPath = Config::get<nlohmann::json::string_t>("/launcherPath", "");
        } else {
            std::wcout << L"Launcher path not given. Exiting..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));
            return 1;
        }
    }

    if (!startLauncher(launcherPath)) {
        return 1;
    }

    std::cout << "Launcher has been started. Please start the game from the launcher." << std::endl;

    blackbone::Process qrslProcess;

    while (true) {
        if (NT_SUCCESS(qrslProcess.Attach(L"QRSL.exe"))) {
            break;
        }

        std::cout << "Waiting for QRSL.exe..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    std::cout << "Wait until you're at the login screen, then press F1 to inject the DLL." << std::endl;

    while (true) {
        if (GetAsyncKeyState(VK_F1) & 1) {
            break;
        }
    }

    std::wcout << L"Injecting " + directory + L"TOFInternal.dll" << std::endl;

    auto callback = [](blackbone::CallbackType type, void *ctx, blackbone::Process &proc,
                       const blackbone::ModuleData &modInfo) {
        const auto func = proc.modules().GetExport(L"TOFInternal.dll", "setDirectory");

        if (func.success()) {
            const auto procAddress = (setDirectory)func.result().procAddress;
            setDirectory(directory);
        } else {
            std::cout << "Failed to get setDirectory function. Status: " << func.status << std::endl;
        }

        return blackbone::LoadData::LoadData(blackbone::MappingType::MT_Default, blackbone::LdrRefFlags::Ldr_None);
    };

    const auto result = qrslProcess.mmap().MapImage(
        directory + L"TOFInternal.dll",
        blackbone::eLoadFlags::NoThreads | blackbone::eLoadFlags::ManualImports | blackbone::eLoadFlags::RebaseProcess,
        callback);

    if (result.success()) {
        std::cout << "Injected successfully!" << std::endl;
    } else {
        std::cout << "Failed to inject. Status: " << result.status << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(4));

    Config::shutdown();

    return 0;
}