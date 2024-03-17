int MainThread(HINSTANCE hInstDLL) { return 0; }

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        HANDLE thread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hInstDLL, 0, nullptr);
        if (thread) {
            CloseHandle(thread);
        }
    }

    return true;
}