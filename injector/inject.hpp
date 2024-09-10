HANDLE InjectDll(HANDLE proc, const std::wstring dll) {
    const auto dllAddr = VirtualAllocEx(proc, nullptr, dll.size() * sizeof(wchar_t), MEM_COMMIT, PAGE_READWRITE);

    if (!dllAddr) {
        std::cout << "Failed to allocate memory for DLL path" << std::endl;
        return 0;
    }

    if (!WriteProcessMemory(proc, dllAddr, dll.c_str(), dll.size() * sizeof(wchar_t), nullptr)) {
        std::cout << "Failed to write DLL path into memory" << std::endl;
        return 0;
    }

    const auto loadLib = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

    const auto thread =
        CreateRemoteThreadEx(proc, nullptr, 0, (PTHREAD_START_ROUTINE)loadLib, dllAddr, 0, nullptr, nullptr);

    if (!thread) {
        std::cout << "Failed to create remote thread" << std::endl;
        return thread;
    }

    std::cout << "Created remote thread for loading DLL" << std::endl;

    return thread;
}