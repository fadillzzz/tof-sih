using f_LoadLibraryA = HINSTANCE(WINAPI *)(const char *lpLibFilename);
using f_GetProcAddress = FARPROC(WINAPI *)(HMODULE hModule, LPCSTR lpProcName);
using f_DLL_ENTRY_POINT = BOOL(WINAPI *)(void *hDll, DWORD dwReason, void *pReserved);

#ifdef _WIN64
using f_RtlAddFunctionTable = BOOL(WINAPIV *)(PRUNTIME_FUNCTION FunctionTable, DWORD EntryCount, DWORD64 BaseAddress);
#endif

struct MANUAL_MAPPING_DATA {
    f_LoadLibraryA pLoadLibraryA;
    f_GetProcAddress pGetProcAddress;
#ifdef _WIN64
    f_RtlAddFunctionTable pRtlAddFunctionTable;
#endif
    BYTE *pbase;
    HINSTANCE hMod;
    DWORD fdwReasonParam;
    LPVOID reservedParam;
    BOOL SEHSupport;
};

// Note: Exception support only x64 with build params /EHa or /EHc
bool ManualMapDll(HANDLE hProc, BYTE *pSrcData, SIZE_T FileSize, bool ClearHeader = true,
                  bool ClearNonNeededSections = true, bool AdjustProtections = true, bool SEHExceptionSupport = true,
                  DWORD fdwReason = DLL_PROCESS_ATTACH, LPVOID lpReserved = 0);
void __stdcall loader(MANUAL_MAPPING_DATA *pData);

#if defined(DISABLE_OUTPUT)
#define ILog(data, ...)
#else
#define ILog(text, ...) printf(text, __VA_ARGS__);
#endif

#ifdef _WIN64
#define CURRENT_ARCH IMAGE_FILE_MACHINE_AMD64
#else
#define CURRENT_ARCH IMAGE_FILE_MACHINE_I386
#endif

bool ManualMapDll(HANDLE hProc, BYTE *pSrcData, SIZE_T FileSize, bool ClearHeader, bool ClearNonNeededSections,
                  bool AdjustProtections, bool SEHExceptionSupport, DWORD fdwReason, LPVOID lpReserved) {
    IMAGE_NT_HEADERS *pOldNtHeader = nullptr;
    IMAGE_OPTIONAL_HEADER *pOldOptHeader = nullptr;
    IMAGE_FILE_HEADER *pOldFileHeader = nullptr;
    BYTE *pTargetBase = nullptr;

    if (reinterpret_cast<IMAGE_DOS_HEADER *>(pSrcData)->e_magic != 0x5A4D) { //"MZ"
        ILog("Invalid file\n");
        return false;
    }

    pOldNtHeader =
        reinterpret_cast<IMAGE_NT_HEADERS *>(pSrcData + reinterpret_cast<IMAGE_DOS_HEADER *>(pSrcData)->e_lfanew);
    pOldOptHeader = &pOldNtHeader->OptionalHeader;
    pOldFileHeader = &pOldNtHeader->FileHeader;

    if (pOldFileHeader->Machine != CURRENT_ARCH) {
        ILog("Invalid platform\n");
        return false;
    }

    ILog("File ok\n");

    pTargetBase = reinterpret_cast<BYTE *>(
        VirtualAllocEx(hProc, nullptr, pOldOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!pTargetBase) {
        ILog("Target process memory allocation failed (ex) 0x%X\n", GetLastError());
        return false;
    }

    DWORD oldp = 0;
    VirtualProtectEx(hProc, pTargetBase, pOldOptHeader->SizeOfImage, PAGE_EXECUTE_READWRITE, &oldp);

    MANUAL_MAPPING_DATA data{0};
    data.pLoadLibraryA = LoadLibraryA;
    data.pGetProcAddress = GetProcAddress;
#ifdef _WIN64
    data.pRtlAddFunctionTable = (f_RtlAddFunctionTable)RtlAddFunctionTable;
#else
    SEHExceptionSupport = false;
#endif
    data.pbase = pTargetBase;
    data.fdwReasonParam = fdwReason;
    data.reservedParam = lpReserved;
    data.SEHSupport = SEHExceptionSupport;

    // File header
    if (!WriteProcessMemory(hProc, pTargetBase, pSrcData, 0x1000, nullptr)) { // only first 0x1000 bytes for the header
        ILog("Can't write file header 0x%X\n", GetLastError());
        VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        return false;
    }

    IMAGE_SECTION_HEADER *pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
    for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
        if (pSectionHeader->SizeOfRawData) {
            if (!WriteProcessMemory(hProc, pTargetBase + pSectionHeader->VirtualAddress,
                                    pSrcData + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData,
                                    nullptr)) {
                ILog("Can't map sections: 0x%x\n", GetLastError());
                VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
                return false;
            }
        }
    }

    // Mapping params
    BYTE *MappingDataAlloc = reinterpret_cast<BYTE *>(
        VirtualAllocEx(hProc, nullptr, sizeof(MANUAL_MAPPING_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!MappingDataAlloc) {
        ILog("Target process mapping allocation failed (ex) 0x%X\n", GetLastError());
        VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        return false;
    }

    if (!WriteProcessMemory(hProc, MappingDataAlloc, &data, sizeof(MANUAL_MAPPING_DATA), nullptr)) {
        ILog("Can't write mapping 0x%X\n", GetLastError());
        VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
        return false;
    }

    // Shell code
    void *pShellcode = VirtualAllocEx(hProc, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!pShellcode) {
        ILog("Memory shellcode allocation failed (ex) 0x%X\n", GetLastError());
        VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
        return false;
    }

    if (!WriteProcessMemory(hProc, pShellcode, loader, 0x1000, nullptr)) {
        ILog("Can't write shellcode 0x%X\n", GetLastError());
        VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
        VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
        return false;
    }

    ILog("Mapped DLL at %p\n", pTargetBase);
    ILog("Mapping info at %p\n", MappingDataAlloc);
    ILog("Shell code at %p\n", pShellcode);

    ILog("Data allocated\n");

#ifdef _DEBUG
    ILog("My shellcode pointer %p\n", Shellcode);
    ILog("Target point %p\n", pShellcode);
    system("pause");
#endif

    THREADENTRY32 TE32{0};
    TE32.dwSize = sizeof(TE32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetProcessId(hProc));
    if (hSnap == INVALID_HANDLE_VALUE) {

        return false;
    }

    DWORD dwTargetPID = GetProcessId(hProc);
    DWORD ThreadID = 0;

    BOOL bRet = Thread32First(hSnap, &TE32);
    if (!bRet) {

        CloseHandle(hSnap);

        return false;
    }

    do {
        if (TE32.th32OwnerProcessID == dwTargetPID) {
            ThreadID = TE32.th32ThreadID;
            break;
        }

        bRet = Thread32Next(hSnap, &TE32);
    } while (bRet);

    if (!ThreadID) {
        return false;
    }

    HANDLE hThread = OpenThread(THREAD_SET_CONTEXT | THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, ThreadID);
    if (!hThread) {

        return false;
    }

    if (SuspendThread(hThread) == (DWORD)-1) {

        CloseHandle(hThread);

        return false;
    }

    CONTEXT OldContext{0};
    OldContext.ContextFlags = CONTEXT_CONTROL;

    if (!GetThreadContext(hThread, &OldContext)) {

        ResumeThread(hThread);
        CloseHandle(hThread);

        return false;
    }

    void *pCodecave = VirtualAllocEx(hProc, nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!pCodecave) {

        ResumeThread(hThread);
        CloseHandle(hThread);

        return false;
    }

#ifdef _WIN64

    BYTE Shellcode[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // - 0x08			-> returned value

        0x48, 0x83, 0xEC, 0x08, // + 0x00			-> sub rsp, 0x08

        0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00,       // + 0x04 (+ 0x07)	-> mov [rsp], RipLowPart
        0xC7, 0x44, 0x24, 0x04, 0x00, 0x00, 0x00, 0x00, // + 0x0B (+ 0x0F)	-> mov [rsp + 0x04], RipHighPart

        0x50, 0x51, 0x52, 0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41,
        0x53, // + 0x13			-> push r(a/c/d)x / r(8 - 11)
        0x9C, // + 0x1E			-> pushfq

        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // + 0x1F (+ 0x21)	-> mov rax, pRoutine
        0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // + 0x29 (+ 0x2B)	-> mov rcx, pArg

        0x48, 0x83, 0xEC, 0x20, // + 0x33			-> sub rsp, 0x20
        0xFF, 0xD0,             // + 0x37			-> call rax
        0x48, 0x83, 0xC4, 0x20, // + 0x39			-> add rsp, 0x20

        0x48, 0x8D, 0x0D, 0xB4, 0xFF, 0xFF, 0xFF, // + 0x3D			-> lea rcx, [pCodecave]
        0x48, 0x89, 0x01,                         // + 0x44			-> mov [rcx], rax

        0x9D, // + 0x47			-> popfq
        0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58, 0x5A, 0x59,
        0x58, // + 0x48			-> pop r(11-8) / r(d/c/a)x

        0xC6, 0x05, 0xA9, 0xFF, 0xFF, 0xFF, 0x00, // + 0x53			-> mov byte ptr[$ - 0x57], 0

        0xC3 // + 0x5A			-> ret
    }; // SIZE = 0x5B (+ 0x08)

    DWORD FuncOffset = 0x08;
    DWORD CheckByteOffset = 0x03 + FuncOffset;

    DWORD dwLoRIP = (DWORD)(OldContext.Rip & 0xFFFFFFFF);
    DWORD dwHiRIP = (DWORD((OldContext.Rip) >> 0x20) & 0xFFFFFFFF);

    *reinterpret_cast<DWORD *>(Shellcode + 0x07 + FuncOffset) = dwLoRIP;
    *reinterpret_cast<DWORD *>(Shellcode + 0x0F + FuncOffset) = dwHiRIP;

    *reinterpret_cast<void **>(Shellcode + 0x21 + FuncOffset) = pShellcode;
    *reinterpret_cast<void **>(Shellcode + 0x2B + FuncOffset) = MappingDataAlloc;

    OldContext.Rip = reinterpret_cast<UINT_PTR>(pCodecave) + FuncOffset;

#else

    BYTE Shellcode[] = {
        0x00, 0x00, 0x00, 0x00, // - 0x04 (pCodecave)	-> returned value
                                // ;buffer to store returned value (eax)

        0x83, 0xEC, 0x04, // + 0x00				-> sub esp, 0x04
                          // ;prepare stack for ret
        0xC7, 0x04, 0x24, 0x00, 0x00, 0x00,
        0x00, // + 0x03 (+ 0x06)		-> mov [esp], OldEip						;store
              // old eip as return address

        0x50, 0x51, 0x52, // + 0x0A				-> psuh e(a/c/d)
                          // ;save e(a/c/d)x
        0x9C,             // + 0x0D				-> pushfd
                          // ;save flags register

        0xB9, 0x00, 0x00, 0x00, 0x00, // + 0x0E (+ 0x0F)		-> mov ecx, pArg
                                      // ;load pArg into ecx
        0xB8, 0x00, 0x00, 0x00, 0x00, // + 0x13 (+ 0x14)		-> mov eax, pRoutine

        0x51,       // + 0x18				-> push ecx
                    // ;push pArg
        0xFF, 0xD0, // + 0x19				-> call eax
                    // ;call target function

        0xA3, 0x00, 0x00, 0x00, 0x00, // + 0x1B (+ 0x1C)		-> mov dword ptr[pCodecave], eax
                                      // ;store returned value

        0x9D,             // + 0x20				-> popfd
                          // ;restore flags register
        0x5A, 0x59, 0x58, // + 0x21				-> pop e(d/c/a)
                          // ;restore e(d/c/a)x

        0xC6, 0x05, 0x00, 0x00, 0x00, 0x00,
        0x00, // + 0x24 (+ 0x26)		-> mov byte ptr[pCodecave + 0x06], 0x00		;set checkbyte to 0

        0xC3 // + 0x2B				-> ret
             // ;return to OldEip
    }; // SIZE = 0x2C (+ 0x04)

    DWORD FuncOffset = 0x04;
    DWORD CheckByteOffset = 0x02 + FuncOffset;

    *reinterpret_cast<DWORD *>(Shellcode + 0x06 + FuncOffset) = OldContext.Eip;

    *reinterpret_cast<void **>(Shellcode + 0x0F + FuncOffset) = pArg;
    *reinterpret_cast<void **>(Shellcode + 0x14 + FuncOffset) = pRoutine;

    *reinterpret_cast<void **>(Shellcode + 0x1C + FuncOffset) = pCodecave;
    *reinterpret_cast<BYTE **>(Shellcode + 0x26 + FuncOffset) = reinterpret_cast<BYTE *>(pCodecave) + CheckByteOffset;

    OldContext.Eip = reinterpret_cast<DWORD>(pCodecave) + FuncOffset;

#endif

    if (!WriteProcessMemory(hProc, pCodecave, Shellcode, sizeof(Shellcode), nullptr)) {

        ResumeThread(hThread);
        CloseHandle(hThread);
        VirtualFreeEx(hProc, pCodecave, 0, MEM_RELEASE);

        return false;
    }

    if (!SetThreadContext(hThread, &OldContext)) {

        ResumeThread(hThread);
        CloseHandle(hThread);
        VirtualFreeEx(hProc, pCodecave, 0, MEM_RELEASE);

        return false;
    }

    if (ResumeThread(hThread) == (DWORD)-1) {

        CloseHandle(hThread);
        VirtualFreeEx(hProc, pCodecave, 0, MEM_RELEASE);

        return false;
    }

    CloseHandle(hThread);

    ILog("Thread created at: %p, waiting for return...\n", pShellcode);

    HINSTANCE hCheck = NULL;
    while (!hCheck) {
        DWORD exitcode = 0;
        GetExitCodeProcess(hProc, &exitcode);
        if (exitcode != STILL_ACTIVE) {
            ILog("Process crashed, exit code: %d\n", exitcode);
            return false;
        }

        MANUAL_MAPPING_DATA data_checked{0};
        ReadProcessMemory(hProc, MappingDataAlloc, &data_checked, sizeof(data_checked), nullptr);
        hCheck = data_checked.hMod;

        if (hCheck == (HINSTANCE)0x404040) {
            ILog("Wrong mapping ptr\n");
            VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
            VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
            VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
            return false;
        } else if (hCheck == (HINSTANCE)0x505050) {
            ILog("WARNING: Exception support failed!\n");
        }

        Sleep(10);
    }

    BYTE *emptyBuffer = (BYTE *)malloc(1024 * 1024 * 20);
    if (emptyBuffer == nullptr) {
        ILog("Unable to allocate memory\n");
        return false;
    }
    memset(emptyBuffer, 0, 1024 * 1024 * 20);

    // CLEAR PE HEAD
    if (ClearHeader) {
        if (!WriteProcessMemory(hProc, pTargetBase, emptyBuffer, 0x1000, nullptr)) {
            ILog("WARNING!: Can't clear HEADER\n");
        }
    }
    // END CLEAR PE HEAD

    if (ClearNonNeededSections) {
        pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
        for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
            if (pSectionHeader->Misc.VirtualSize) {
                if ((SEHExceptionSupport ? 0 : strcmp((char *)pSectionHeader->Name, ".pdata") == 0) ||
                    strcmp((char *)pSectionHeader->Name, ".rsrc") == 0 ||
                    strcmp((char *)pSectionHeader->Name, ".reloc") == 0) {
                    ILog("Processing %s removal\n", pSectionHeader->Name);
                    if (!WriteProcessMemory(hProc, pTargetBase + pSectionHeader->VirtualAddress, emptyBuffer,
                                            pSectionHeader->Misc.VirtualSize, nullptr)) {
                        ILog("Can't clear section %s: 0x%x\n", pSectionHeader->Name, GetLastError());
                    }
                }
            }
        }
    }

    if (AdjustProtections) {
        pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
        for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
            if (pSectionHeader->Misc.VirtualSize) {
                DWORD old = 0;
                DWORD newP = PAGE_READONLY;

                if ((pSectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE) > 0) {
                    newP = PAGE_READWRITE;
                } else if ((pSectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) > 0) {
                    newP = PAGE_EXECUTE_READ;
                }
                if (VirtualProtectEx(hProc, pTargetBase + pSectionHeader->VirtualAddress,
                                     pSectionHeader->Misc.VirtualSize, newP, &old)) {
                    ILog("section %s set as %lX\n", (char *)pSectionHeader->Name, newP);
                } else {
                    ILog("FAIL: section %s not set as %lX\n", (char *)pSectionHeader->Name, newP);
                }
            }
        }
        DWORD old = 0;
        VirtualProtectEx(hProc, pTargetBase, IMAGE_FIRST_SECTION(pOldNtHeader)->VirtualAddress, PAGE_READONLY, &old);
    }

    if (!WriteProcessMemory(hProc, pShellcode, emptyBuffer, 0x1000, nullptr)) {
        ILog("WARNING: Can't clear shellcode\n");
    }
    if (!VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE)) {
        ILog("WARNING: can't release shell code memory\n");
    }
    if (!VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE)) {
        ILog("WARNING: can't release mapping data memory\n");
    }

    return true;
}

#define RELOC_FLAG32(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)

#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#else
#define RELOC_FLAG RELOC_FLAG32
#endif

#pragma runtime_checks("", off)
#pragma optimize("", off)
void __stdcall loader(MANUAL_MAPPING_DATA *pData) {
    if (!pData) {
        pData->hMod = (HINSTANCE)0x404040;
        return;
    }

    BYTE *pBase = pData->pbase;
    auto *pOpt =
        &reinterpret_cast<IMAGE_NT_HEADERS *>(pBase + reinterpret_cast<IMAGE_DOS_HEADER *>((uintptr_t)pBase)->e_lfanew)
             ->OptionalHeader;

    auto _LoadLibraryA = pData->pLoadLibraryA;
    auto _GetProcAddress = pData->pGetProcAddress;
#ifdef _WIN64
    auto _RtlAddFunctionTable = pData->pRtlAddFunctionTable;
#endif
    auto _DllMain = reinterpret_cast<f_DLL_ENTRY_POINT>(pBase + pOpt->AddressOfEntryPoint);

    BYTE *LocationDelta = pBase - pOpt->ImageBase;
    if (LocationDelta) {
        if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
            auto *pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION *>(
                pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
            const auto *pRelocEnd = reinterpret_cast<IMAGE_BASE_RELOCATION *>(
                reinterpret_cast<uintptr_t>(pRelocData) + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);
            while (pRelocData < pRelocEnd && pRelocData->SizeOfBlock) {
                UINT AmountOfEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                WORD *pRelativeInfo = reinterpret_cast<WORD *>(pRelocData + 1);

                for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo) {
                    if (RELOC_FLAG(*pRelativeInfo)) {
                        UINT_PTR *pPatch = reinterpret_cast<UINT_PTR *>(pBase + pRelocData->VirtualAddress +
                                                                        ((*pRelativeInfo) & 0xFFF));
                        *pPatch += reinterpret_cast<UINT_PTR>(LocationDelta);
                    }
                }
                pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION *>(reinterpret_cast<BYTE *>(pRelocData) +
                                                                       pRelocData->SizeOfBlock);
            }
        }
    }

    if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
        auto *pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(
            pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        while (pImportDescr->Name) {
            char *szMod = reinterpret_cast<char *>(pBase + pImportDescr->Name);
            HINSTANCE hDll = _LoadLibraryA(szMod);

            ULONG_PTR *pThunkRef = reinterpret_cast<ULONG_PTR *>(pBase + pImportDescr->OriginalFirstThunk);
            ULONG_PTR *pFuncRef = reinterpret_cast<ULONG_PTR *>(pBase + pImportDescr->FirstThunk);

            if (!pThunkRef)
                pThunkRef = pFuncRef;

            for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
                if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
                    *pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, reinterpret_cast<char *>(*pThunkRef & 0xFFFF));
                } else {
                    auto *pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME *>(pBase + (*pThunkRef));
                    *pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, pImport->Name);
                }
            }
            ++pImportDescr;
        }
    }

    if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
        auto *pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY *>(
            pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        auto *pCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK *>(pTLS->AddressOfCallBacks);
        for (; pCallback && *pCallback; ++pCallback)
            (*pCallback)(pBase, DLL_PROCESS_ATTACH, nullptr);
    }

    bool ExceptionSupportFailed = false;

#ifdef _WIN64

    if (pData->SEHSupport) {
        auto excep = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
        if (excep.Size) {
            if (!_RtlAddFunctionTable(reinterpret_cast<IMAGE_RUNTIME_FUNCTION_ENTRY *>(pBase + excep.VirtualAddress),
                                      excep.Size / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY), (DWORD64)pBase)) {
                ExceptionSupportFailed = true;
            }
        }
    }

#endif

    _DllMain(pBase, pData->fdwReasonParam, pData->reservedParam);

    if (ExceptionSupportFailed)
        pData->hMod = reinterpret_cast<HINSTANCE>(0x505050);
    else
        pData->hMod = reinterpret_cast<HINSTANCE>(pBase);
}