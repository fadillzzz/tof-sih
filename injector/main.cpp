#include "Process/Process.h"

int main() {
    blackbone::Process process;

    PROCESS_INFORMATION pi;
    SetEnvironmentVariable(L"__COMPAT_LAYER", L"RUNASINVOKER");

    NTSTATUS status =
        process.CreateAndAttach(L"QRSL.exe", true, true, L"", L"D:\\Program Files\\Tower Of Fantasy\\launcher");

    if (!NT_SUCCESS(status)) {
        printf("Failed to create process. Status: 0x%X\n", status);

        getchar();

        return 1;
    }

    // Injection here

    process.Resume();

    return 0;
}