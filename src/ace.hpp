#include "minhook/include/MinHook.h"

namespace Ace {
    int myAce() { return 0; }

    typedef FARPROC(WINAPI *getProcAddressForCaller_t)(HMODULE hModule, LPCSTR lpProcName, void *caller);
    getProcAddressForCaller_t oGetProcAddress = nullptr;
    FARPROC WINAPI getProcAddress(HMODULE hModule, LPCSTR lpProcName, void *caller) {
        if ((uintptr_t)lpProcName >= 0xFFFF) {
            if (std::string(lpProcName).contains("InitAceClient")) {
                return (FARPROC)&myAce;
            }
        }

        return oGetProcAddress(hModule, lpProcName, caller);
    }

    typedef HANDLE(WINAPI *loadLibraryW_t)(LPCWSTR lpLibFileName);
    loadLibraryW_t oLoadLibraryW = nullptr;
    HANDLE WINAPI loadLibraryW(LPCWSTR lpLibFileName) {
        if (std::wstring(lpLibFileName).contains(L"ACE-Base64.dll")) {
            return (HANDLE)0xBAADC0DE;
        }

        return oLoadLibraryW(lpLibFileName);
    }

    void init() {
        MH_Initialize();
        const auto mod = GetModuleHandle(L"kernelbase.dll");

        void *loadLibraryWAddr = GetProcAddress(mod, "LoadLibraryW");
        void *getProcAddressAddr = GetProcAddress(mod, "GetProcAddressForCaller");

        MH_CreateHook(loadLibraryWAddr, loadLibraryW, (void **)&oLoadLibraryW);
        MH_EnableHook(loadLibraryWAddr);

        MH_CreateHook(getProcAddressAddr, getProcAddress, (void **)&oGetProcAddress);
        MH_EnableHook(getProcAddressAddr);
    }
} // namespace Ace
