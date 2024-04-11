#ifdef _WIN64
using f_Routine = UINT_PTR(__fastcall *)(void *pArg);
#else
using f_Routine = UINT_PTR(__stdcall *)(void *pArg);
#endif

#ifdef UNICODE
#define LOAD_LIBRARY_NAME "LoadLibraryW"
#else
#define LOAD_LIBRARY_NAME "LoadLibraryA"
#endif

struct HookData {
    HHOOK m_hHook;
    HWND m_hWnd;
};

struct EnumWindowsCallback_Data {
    std::vector<HookData> m_HookData;
    DWORD m_PID;
    HOOKPROC m_pHook;
    HINSTANCE m_hModule;
};

HINSTANCE GetModuleHandleEx(HANDLE hTargetProc, const TCHAR *lpModuleName) {
    MODULEENTRY32 ME32{0};
    ME32.dwSize = sizeof(ME32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(hTargetProc));
    if (hSnap == INVALID_HANDLE_VALUE) {
        while (GetLastError() == ERROR_BAD_LENGTH) {
            hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(hTargetProc));
            if (hSnap != INVALID_HANDLE_VALUE)
                break;
        }
    }

    if (hSnap == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    BOOL bRet = Module32First(hSnap, &ME32);
    do {
        if (!_tcsicmp(lpModuleName, ME32.szModule))
            break;

        bRet = Module32Next(hSnap, &ME32);

    } while (bRet);

    CloseHandle(hSnap);

    if (!bRet) {
        return NULL;
    }

    return ME32.hModule;
}

void *GetProcAddressEx(HANDLE hTargetProc, const TCHAR *lpModuleName, const char *lpProcName) {
    BYTE *modBase = reinterpret_cast<BYTE *>(GetModuleHandleEx(hTargetProc, lpModuleName));
    if (!modBase)
        return nullptr;

    BYTE *pe_header = new BYTE[0x1000];
    if (!pe_header)
        return nullptr;

    if (!ReadProcessMemory(hTargetProc, modBase, pe_header, 0x1000, nullptr)) {
        delete[] pe_header;

        return nullptr;
    }

    auto *pNT =
        reinterpret_cast<IMAGE_NT_HEADERS *>(pe_header + reinterpret_cast<IMAGE_DOS_HEADER *>(pe_header)->e_lfanew);
    auto *pExportEntry = &pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

    if (!pExportEntry->Size) {
        delete[] pe_header;

        return nullptr;
    }

    BYTE *export_data = new BYTE[pExportEntry->Size];
    if (!export_data) {
        delete[] pe_header;

        return nullptr;
    }

    if (!ReadProcessMemory(hTargetProc, modBase + pExportEntry->VirtualAddress, export_data, pExportEntry->Size,
                           nullptr)) {
        delete[] export_data;
        delete[] pe_header;

        return nullptr;
    }

    BYTE *localBase = export_data - pExportEntry->VirtualAddress;
    auto *pExportDir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY *>(export_data);

    auto Forward = [&](DWORD FuncRVA) -> void * {
        char pFullExport[MAX_PATH + 1]{0};
        auto Len = strlen(reinterpret_cast<char *>(localBase + FuncRVA));
        if (!Len)
            return nullptr;

        memcpy(pFullExport, reinterpret_cast<char *>(localBase + FuncRVA), Len);

        char *pFuncName = strchr(pFullExport, '.');
        *(pFuncName++) = 0;
        if (*pFuncName == '#')
            pFuncName = reinterpret_cast<char *>(LOWORD(atoi(++pFuncName)));

#ifdef UNICODE
        TCHAR ModNameW[MAX_PATH + 1]{0};
        size_t SizeOut = 0;
        mbstowcs_s(&SizeOut, ModNameW, pFullExport, MAX_PATH);

        return GetProcAddressEx(hTargetProc, ModNameW, pFuncName);
#else

        return GetProcAddressEx(hTargetProc, pFullExport, pFuncName);
#endif
    };

    if ((reinterpret_cast<UINT_PTR>(lpProcName) & 0xFFFFFF) <= MAXWORD) {
        WORD Base = LOWORD(pExportDir->Base - 1);
        WORD Ordinal = LOWORD(lpProcName) - Base;
        DWORD FuncRVA = reinterpret_cast<DWORD *>(localBase + pExportDir->AddressOfFunctions)[Ordinal];

        delete[] export_data;
        delete[] pe_header;

        if (FuncRVA >= pExportEntry->VirtualAddress && FuncRVA < pExportEntry->VirtualAddress + pExportEntry->Size) {
            return Forward(FuncRVA);
        }

        return modBase + FuncRVA;
    }

    DWORD max = pExportDir->NumberOfNames - 1;
    DWORD min = 0;
    DWORD FuncRVA = 0;

    while (min <= max) {
        DWORD mid = (min + max) / 2;

        DWORD CurrNameRVA = reinterpret_cast<DWORD *>(localBase + pExportDir->AddressOfNames)[mid];
        char *szName = reinterpret_cast<char *>(localBase + CurrNameRVA);

        int cmp = strcmp(szName, lpProcName);
        if (cmp < 0)
            min = mid + 1;
        else if (cmp > 0)
            max = mid - 1;
        else {
            WORD Ordinal = reinterpret_cast<WORD *>(localBase + pExportDir->AddressOfNameOrdinals)[mid];
            FuncRVA = reinterpret_cast<DWORD *>(localBase + pExportDir->AddressOfFunctions)[Ordinal];

            break;
        }
    }

    delete[] export_data;
    delete[] pe_header;

    if (!FuncRVA)
        return nullptr;

    if (FuncRVA >= pExportEntry->VirtualAddress && FuncRVA < pExportEntry->VirtualAddress + pExportEntry->Size) {
        return Forward(FuncRVA);
    }

    return modBase + FuncRVA;
}

bool SR_SetWindowsHookEx(HANDLE hTargetProc, f_Routine *pRoutine, void *pArg, DWORD &LastWin32Error, UINT_PTR &Out) {
    void *pCodecave = VirtualAllocEx(hTargetProc, nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!pCodecave) {
        LastWin32Error = GetLastError();

        return false;
    }

    void *pCallNextHookEx = GetProcAddressEx(hTargetProc, TEXT("user32.dll"), "CallNextHookEx");
    if (!pCallNextHookEx) {
        VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

        return false;
    }

#ifdef _WIN64

    BYTE Shellcode[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // - 0x18	-> pArg / returned value / rax
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // - 0x10	-> pRoutine
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // - 0x08	-> CallNextHookEx

        0x55, // + 0x00	-> push rbp
        0x54, // + 0x01	-> push rsp
        0x53, // + 0x02	-> push rbx

        0x48, 0x8D, 0x1D, 0xDE, 0xFF, 0xFF, 0xFF, // + 0x03	-> lea rbx, [pArg]

        0x48, 0x83, 0xEC, 0x20, // + 0x0A	-> sub rsp, 0x20
        0x4D, 0x8B, 0xC8,       // + 0x0E	-> mov r9,r8
        0x4C, 0x8B, 0xC2,       // + 0x11	-> mov r8, rdx
        0x48, 0x8B, 0xD1,       // + 0x14	-> mov rdx,rcx
        0xFF, 0x53, 0x10,       // + 0x17	-> call [rbx + 0x10]
        0x48, 0x83, 0xC4, 0x20, // + 0x1A	-> add rsp, 0x20

        0x48, 0x8B, 0xC8, // + 0x1E	-> mov rcx, rax

        0xEB, 0x00,                               // + 0x21	-> jmp $ + 0x02
        0xC6, 0x05, 0xF8, 0xFF, 0xFF, 0xFF, 0x18, // + 0x23	-> mov byte ptr[$ - 0x01], 0x1A

        0x48, 0x87, 0x0B,       // + 0x2A	-> xchg [rbx], rcx
        0x48, 0x83, 0xEC, 0x20, // + 0x2D	-> sub rsp, 0x20
        0xFF, 0x53, 0x08,       // + 0x31	-> call [rbx + 0x08]
        0x48, 0x83, 0xC4, 0x20, // + 0x34	-> add rsp, 0x20

        0x48, 0x87, 0x03, // + 0x38	-> xchg [rbx], rax

        0x5B, // + 0x3B	-> pop rbx
        0x5C, // + 0x3C	-> pop rsp
        0x5D, // + 0x3D	-> pop rbp

        0xC3 // + 0x3E	-> ret
    }; // SIZE = 0x3F (+ 0x18)

    DWORD CodeOffset = 0x18;
    DWORD CheckByteOffset = 0x22 + CodeOffset;

    *reinterpret_cast<void **>(Shellcode + 0x00) = pArg;
    *reinterpret_cast<void **>(Shellcode + 0x08) = pRoutine;
    *reinterpret_cast<void **>(Shellcode + 0x10) = pCallNextHookEx;

#else

    BYTE Shellcode[] = {
        0x00, 0x00, 0x00, 0x00, // - 0x08				-> pArg
        0x00, 0x00, 0x00, 0x00, // - 0x04				-> pRoutine

        0x55,       // + 0x00				-> push ebp
        0x8B, 0xEC, // + 0x01				-> mov ebp, esp

        0xFF, 0x75, 0x10,             // + 0x03				-> push [ebp + 0x10]
        0xFF, 0x75, 0x0C,             // + 0x06				-> push [ebp + 0x0C]
        0xFF, 0x75, 0x08,             // + 0x09				-> push [ebp + 0x08]
        0x6A, 0x00,                   // + 0x0C				-> push 0x00
        0xE8, 0x00, 0x00, 0x00, 0x00, // + 0x0E (+ 0x0F)		-> call CallNextHookEx

        0xEB, 0x00, // + 0x13				-> jmp $ + 0x02

        0x50, // + 0x15				-> push eax
        0x53, // + 0x16				-> push ebx

        0xBB, 0x00, 0x00, 0x00, 0x00, // + 0x17 (+ 0x18)		-> mov ebx, pArg
        0xC6, 0x43, 0x1C, 0x14,       // + 0x1C				-> mov [ebx + 0x1C], 0x17

        0xFF, 0x33, // + 0x20				-> push [ebx]

        0xFF, 0x53, 0x04, // + 0x22				-> call [ebx + 0x04]

        0x89, 0x03, // + 0x25				-> mov [ebx], eax

        0x5B, // + 0x27				-> pop ebx
        0x58, // + 0x28				-> pop eax

        0x5D,            // + 0x29				-> pop ebp
        0xC2, 0x0C, 0x00 // + 0x2A				-> ret 0x000C
    }; // SIZE = 0x3D (+ 0x08)

    DWORD CodeOffset = 0x08;
    DWORD CheckByteOffset = 0x14 + CodeOffset;

    *reinterpret_cast<void **>(Shellcode + 0x00) = pArg;
    *reinterpret_cast<void **>(Shellcode + 0x04) = pRoutine;

    *reinterpret_cast<DWORD *>(Shellcode + 0x0F + CodeOffset) =
        reinterpret_cast<DWORD>(pCallNextHookEx) - (reinterpret_cast<DWORD>(pCodecave) + 0x0E + CodeOffset) - 5;
    *reinterpret_cast<void **>(Shellcode + 0x18 + CodeOffset) = pCodecave;

#endif

    if (!WriteProcessMemory(hTargetProc, pCodecave, Shellcode, sizeof(Shellcode), nullptr)) {
        LastWin32Error = GetLastError();

        VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

        return false;
    }

    static EnumWindowsCallback_Data data;
    data.m_HookData.clear();

    data.m_pHook = reinterpret_cast<HOOKPROC>(reinterpret_cast<BYTE *>(pCodecave) + CodeOffset);
    data.m_PID = GetProcessId(hTargetProc);
    data.m_hModule = GetModuleHandle(TEXT("user32.dll"));

    WNDENUMPROC EnumWindowsCallback = [](HWND hWnd, LPARAM) -> BOOL {
        DWORD winPID = 0;
        DWORD winTID = GetWindowThreadProcessId(hWnd, &winPID);
        if (winPID == data.m_PID) {
            TCHAR szWindow[MAX_PATH]{0};
            if (IsWindowVisible(hWnd) && GetWindowText(hWnd, szWindow, MAX_PATH)) {
                if (GetClassName(hWnd, szWindow, MAX_PATH) && _tcscmp(szWindow, TEXT("ConsoleWindowClass"))) {
                    HHOOK hHook = SetWindowsHookEx(WH_CALLWNDPROC, data.m_pHook, data.m_hModule, winTID);
                    if (hHook) {
                        data.m_HookData.push_back({hHook, hWnd});
                    }
                }
            }
        }

        return TRUE;
    };

    if (!EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&data))) {
        LastWin32Error = GetLastError();

        VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

        return false;
    }

    if (data.m_HookData.empty()) {
        VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

        return false;
    }

    HWND hForegroundWnd = GetForegroundWindow();
    for (auto i : data.m_HookData) {
        SetForegroundWindow(i.m_hWnd);
        SendMessageA(i.m_hWnd, WM_KEYDOWN, VK_SPACE, 0);
        Sleep(10);
        SendMessageA(i.m_hWnd, WM_KEYUP, VK_SPACE, 0);
        UnhookWindowsHookEx(i.m_hHook);
    }
    SetForegroundWindow(hForegroundWnd);

    DWORD Timer = GetTickCount();
    BYTE CheckByte = 0;

    do {
        ReadProcessMemory(hTargetProc, reinterpret_cast<BYTE *>(pCodecave) + CheckByteOffset, &CheckByte, 1, nullptr);

        if (GetTickCount() - Timer > 5000) {
            return false;
        }

        Sleep(10);

    } while (!CheckByte);

    ReadProcessMemory(hTargetProc, pCodecave, &Out, sizeof(Out), nullptr);

    VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

    return true;
}

bool InjectDll(HANDLE hProc, const TCHAR *szPath) {
    if (!hProc) {
        DWORD dwErr = GetLastError();
        printf("OpenProcess failed: 0x%08X\n", dwErr);

        return false;
    }

    auto len = _tcslen(szPath) * sizeof(TCHAR);
    void *pArg = VirtualAllocEx(hProc, nullptr, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!pArg) {
        DWORD dwErr = GetLastError();
        printf("VirtualAllocEx failed: 0x%08X\n", dwErr);

        CloseHandle(hProc);

        return false;
    }

    BOOL bRet = WriteProcessMemory(hProc, pArg, szPath, len, nullptr);
    if (!bRet) {
        DWORD dwErr = GetLastError();
        printf("WriteProcessMemory failed: 0x%08X\n", dwErr);

        VirtualFreeEx(hProc, pArg, 0, MEM_RELEASE);
        CloseHandle(hProc);

        return false;
    }

    f_Routine *p_LoadLibrary =
        reinterpret_cast<f_Routine *>(GetProcAddressEx(hProc, TEXT("kernel32.dll"), LOAD_LIBRARY_NAME));
    if (!p_LoadLibrary) {
        printf("Can't find LoadLibrary\n");

        VirtualFreeEx(hProc, pArg, 0, MEM_RELEASE);
        CloseHandle(hProc);

        return false;
    }

    UINT_PTR hDllOut = 0;
    DWORD last_error = 0;
    bool dwErr = SR_SetWindowsHookEx(hProc, p_LoadLibrary, pArg, last_error, hDllOut);

    CloseHandle(hProc);

    if (!dwErr) {
        printf("StartRoutine failed\n");
        printf("LastWin32Error: 0x%08X\n", last_error);

        return false;
    }

    printf("Success! LoadLibrary returned 0x%p\n", reinterpret_cast<void *>(hDllOut));

    return true;
}
