#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>

HANDLE InjectDll(HANDLE proc, const std::wstring dll);
