// Taken from https://github.com/hrt/ThreadJect-x64/blob/master/ThreadJect/main.cpp

typedef HMODULE(WINAPI *pLoadLibraryA)(LPCSTR);
typedef FARPROC(WINAPI *pGetProcAddress)(HMODULE, LPCSTR);

typedef BOOL(WINAPI *PDLL_MAIN)(HMODULE, DWORD, PVOID);

typedef struct _MANUAL_INJECT {
    PVOID ImageBase;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION BaseRelocation;
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
    pLoadLibraryA fnLoadLibraryA;
    pGetProcAddress fnGetProcAddress;
} MANUAL_INJECT, *PMANUAL_INJECT;

DWORD WINAPI LoadDll(PVOID p) {
    PMANUAL_INJECT ManualInject;

    HMODULE hModule;
    DWORD64 i, Function, count, delta;

    DWORD64 *ptr;
    PWORD list;

    PIMAGE_BASE_RELOCATION pIBR;
    PIMAGE_IMPORT_DESCRIPTOR pIID;
    PIMAGE_IMPORT_BY_NAME pIBN;
    PIMAGE_THUNK_DATA FirstThunk, OrigFirstThunk;

    PDLL_MAIN EntryPoint;

    ManualInject = (PMANUAL_INJECT)p;

    pIBR = ManualInject->BaseRelocation;
    delta = (DWORD64)((LPBYTE)ManualInject->ImageBase -
                      ManualInject->NtHeaders->OptionalHeader.ImageBase); // Calculate the delta

    // Relocate the image

    while (pIBR->VirtualAddress) {
        if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION)) {
            count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            list = (PWORD)(pIBR + 1);

            for (i = 0; i < count; i++) {
                if (list[i]) {
                    ptr = (DWORD64 *)((LPBYTE)ManualInject->ImageBase + (pIBR->VirtualAddress + (list[i] & 0xFFF)));
                    *ptr += delta;
                }
            }
        }

        pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
    }

    pIID = ManualInject->ImportDirectory;

    // Resolve DLL imports

    while (pIID->Characteristics) {
        OrigFirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->OriginalFirstThunk);
        FirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->FirstThunk);

        hModule = ManualInject->fnLoadLibraryA((LPCSTR)ManualInject->ImageBase + pIID->Name);

        if (!hModule) {
            return FALSE;
        }

        while (OrigFirstThunk->u1.AddressOfData) {
            if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
                // Import by ordinal

                Function =
                    (DWORD64)ManualInject->fnGetProcAddress(hModule, (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));

                if (!Function) {
                    return FALSE;
                }

                FirstThunk->u1.Function = Function;
            }

            else {
                // Import by name

                pIBN = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)ManualInject->ImageBase + OrigFirstThunk->u1.AddressOfData);
                Function = (DWORD64)ManualInject->fnGetProcAddress(hModule, (LPCSTR)pIBN->Name);

                if (!Function) {
                    return FALSE;
                }

                FirstThunk->u1.Function = Function;
            }

            OrigFirstThunk++;
            FirstThunk++;
        }

        pIID++;
    }

    if (ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint) {
        EntryPoint =
            (PDLL_MAIN)((LPBYTE)ManualInject->ImageBase + ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint);
        return EntryPoint((HMODULE)ManualInject->ImageBase, DLL_PROCESS_ATTACH, NULL); // Call the entry point
    }

    return TRUE;
}

extern "C" NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);

UCHAR code[] = {
    0x48, 0xB8, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // mov -16 to rax
    0x48, 0x21, 0xC4,                                           // and rsp, rax
    0x48, 0x83, 0xEC, 0x20,                                     // subtract 32 from rsp
    0x48, 0x8b, 0xEC,                                           // mov rbp, rsp
    0x90, 0x90,                                                 // nop nop
    0x48, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // mov rcx,CCCCCCCCCCCCCCCC
    0x48, 0xB8, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, // mov rax,AAAAAAAAAAAAAAAA
    0xFF, 0xD0,                                                 // call rax
    0x90,                                                       // nop
    0x90,                                                       // nop
    0xEB, 0xFC                                                  // JMP to nop
};

bool inject(DWORD processId, const std::wstring &dllPath) {
    LPBYTE ptr;
    HANDLE hProcess, hThread, hSnap, hFile;
    PVOID mem, mem1;
    DWORD ProcessId, FileSize, read, i;
    PVOID buffer, image;
    BOOLEAN bl;
    PIMAGE_DOS_HEADER pIDH;
    PIMAGE_NT_HEADERS pINH;
    PIMAGE_SECTION_HEADER pISH;

    THREADENTRY32 te32;
    CONTEXT ctx;

    MANUAL_INJECT ManualInject;

    te32.dwSize = sizeof(te32);
    ctx.ContextFlags = CONTEXT_FULL;

    hFile = CreateFile(dllPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    FileSize = GetFileSize(hFile, NULL);
    buffer = VirtualAlloc(NULL, FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!buffer) {
        std::cout << "Failed to allocate memory for the buffer." << std::endl;

        CloseHandle(hFile);
        return false;
    }

    if (!ReadFile(hFile, buffer, FileSize, &read, NULL)) {
        std::cout << "Failed to read the DLL." << std::endl;

        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hFile);

        return false;
    }

    CloseHandle(hFile);

    pIDH = (PIMAGE_DOS_HEADER)buffer;

    pINH = (PIMAGE_NT_HEADERS)((LPBYTE)buffer + pIDH->e_lfanew);

    RtlAdjustPrivilege(20, TRUE, FALSE, &bl);

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

    if (!hProcess) {
        std::cout << "Failed to open the process." << std::endl;

        VirtualFree(buffer, 0, MEM_RELEASE);
        return false;
    }

    image = VirtualAllocEx(hProcess, NULL, pINH->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE,
                           PAGE_EXECUTE_READWRITE);

    if (!image) {
        std::cout << "Failed to allocate memory for the image." << std::endl;

        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);

        return false;
    }

    if (!WriteProcessMemory(hProcess, image, buffer, pINH->OptionalHeader.SizeOfHeaders, NULL)) {
        std::cout << "Failed to write the headers." << std::endl;

        VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);

        return false;
    }

    pISH = (PIMAGE_SECTION_HEADER)(pINH + 1);

    for (i = 0; i < pINH->FileHeader.NumberOfSections; i++) {
        WriteProcessMemory(hProcess, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress),
                           (PVOID)((LPBYTE)buffer + pISH[i].PointerToRawData), pISH[i].SizeOfRawData, NULL);
    }

    mem1 = VirtualAllocEx(hProcess, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!mem1) {
        std::cout << "Failed to allocate memory for the loader." << std::endl;

        VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);

        return false;
    }

    memset(&ManualInject, 0, sizeof(MANUAL_INJECT));

    ManualInject.ImageBase = image;
    ManualInject.NtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)image + pIDH->e_lfanew);
    ManualInject.BaseRelocation =
        (PIMAGE_BASE_RELOCATION)((LPBYTE)image +
                                 pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
    ManualInject.ImportDirectory =
        (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)image +
                                   pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    ManualInject.fnLoadLibraryA = LoadLibraryA;
    ManualInject.fnGetProcAddress = GetProcAddress;

    if (!WriteProcessMemory(hProcess, mem1, &ManualInject, sizeof(MANUAL_INJECT), NULL)) {
        std::cout << "Error writing to memory: " << std::hex << GetLastError() << std::endl;
    }

    if (!WriteProcessMemory(hProcess, (PVOID)(PMANUAL_INJECT(mem1) + 1), LoadDll, 0x1000 - sizeof(MANUAL_INJECT),
                            NULL)) {
        std::cout << "Error writing to memory: " << std::hex << GetLastError() << std::endl;
    }

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    Thread32First(hSnap, &te32);

    while (Thread32Next(hSnap, &te32)) {
        if (te32.th32OwnerProcessID == processId) {
            break;
        }
    }

    CloseHandle(hSnap);

    mem = VirtualAllocEx(hProcess, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!mem) {
        std::cout << "Failed to allocate memory for the shellcode." << std::endl;

        VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
        VirtualFreeEx(hProcess, mem1, 0, MEM_RELEASE);
        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);

        return false;
    }

    hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);

    SuspendThread(hThread);
    GetThreadContext(hThread, &ctx);

    buffer = VirtualAlloc(NULL, 65536, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    ptr = (LPBYTE)buffer;
    ZeroMemory(buffer, 65536);

    memcpy(buffer, code, sizeof(code));

    for (BYTE *ptr = (LPBYTE)buffer; ptr < (LPBYTE)buffer + sizeof(code); ptr++) {
        DWORD64 address = *(DWORD64 *)ptr;
        if (address == 0xCCCCCCCCCCCCCCCC) {
            *(DWORD64 *)ptr = (DWORD64)mem1;
        }

        if (address == 0xAAAAAAAAAAAAAAAA) {
            *(DWORD64 *)ptr = (DWORD64)((PMANUAL_INJECT)mem1 + 1);
        }
    }

    if (!WriteProcessMemory(hProcess, mem, buffer, sizeof(code), NULL)) {
        std::cout << "Failed to write the code." << std::endl;

        VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
        VirtualFreeEx(hProcess, mem1, 0, MEM_RELEASE);
        VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
        VirtualFree(buffer, 0, MEM_RELEASE);
        ResumeThread(hThread);
        CloseHandle(hProcess);
        CloseHandle(hThread);

        return false;
    }

    ctx.Rip = (DWORD64)mem;

    if (!SetThreadContext(hThread, &ctx)) {
        std::cout << "Failed to set the thread context." << std::endl;

        VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
        VirtualFreeEx(hProcess, mem1, 0, MEM_RELEASE);
        VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
        VirtualFree(buffer, 0, MEM_RELEASE);
        ResumeThread(hThread);
        CloseHandle(hProcess);
        CloseHandle(hThread);

        return false;
    }

    ResumeThread(hThread);

    CloseHandle(hThread);
    CloseHandle(hProcess);

    VirtualFree(buffer, 0, MEM_RELEASE);

    return true;
}