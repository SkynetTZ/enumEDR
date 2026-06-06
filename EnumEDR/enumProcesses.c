#include "common.h"

BOOL PrintProcesses(IN PSYSTEM_PROCESS_INFORMATION pSystemProcInfo) {

    if (g_Output.mode == OUTPUT_JSON) {
        printf("{\n  \"processes\": [\n");
        BOOL first = TRUE;
        while (TRUE) {
            if (pSystemProcInfo->ImageName.Buffer != NULL) {
                if (!first) printf(",\n");
                first = FALSE;
                printf("    {\"name\": \"");
                for (USHORT i = 0; i < pSystemProcInfo->ImageName.Length / sizeof(WCHAR); i++) {
                    WCHAR c = pSystemProcInfo->ImageName.Buffer[i];
                    if (c == '"' || c == '\\') putchar('\\');
                    if (c < 128) putchar((char)c);
                }
                printf("\", \"pid\": %lu}", (ULONG)(ULONG_PTR)pSystemProcInfo->UniqueProcessId);
            }
            if (!pSystemProcInfo->NextEntryOffset) break;
            pSystemProcInfo = (PSYSTEM_PROCESS_INFORMATION)((ULONG_PTR)pSystemProcInfo + pSystemProcInfo->NextEntryOffset);
        }
        printf("\n  ]\n}\n");
        return TRUE;
    }

    ConsoleSection("Running Processes", "user-mode process inventory");
    ConsoleTableHeader2(L"Process Name", L"PID");

    while (TRUE) {
        WCHAR pidBuf[16];
        swprintf_s(pidBuf, _countof(pidBuf), L"%lu", (ULONG)(ULONG_PTR)pSystemProcInfo->UniqueProcessId);

        if (pSystemProcInfo->ImageName.Buffer != NULL)
            ConsoleTableRow2(pSystemProcInfo->ImageName.Buffer, pidBuf);
        else
            ConsoleTableRow2(L"[Unnamed Process]", pidBuf);

        if (!pSystemProcInfo->NextEntryOffset) break;
        pSystemProcInfo = (PSYSTEM_PROCESS_INFORMATION)((ULONG_PTR)pSystemProcInfo + pSystemProcInfo->NextEntryOffset);
    }

    if (g_Output.useColor)
        wprintf(L"  %ls└──────────────────────────────────────────┴──────────┘%ls\n", L"\x1b[2m", L"\x1b[0m");
    else
        wprintf(L"  +------------------------------------------+----------+\n");

    return TRUE;
}

DWORD CountProcesses(IN PSYSTEM_PROCESS_INFORMATION pSystemProcInfo) {

    DWORD dwProcessCount = 0;

    while (TRUE) {
        dwProcessCount++;
        if (!pSystemProcInfo->NextEntryOffset) break;
        pSystemProcInfo = (PSYSTEM_PROCESS_INFORMATION)((ULONG_PTR)pSystemProcInfo + pSystemProcInfo->NextEntryOffset);
    }

    return dwProcessCount;
}

BOOL EnumerateProcesses(OUT PSYSTEM_PROCESS_INFORMATION* ppSystemProcInfo) {

    BOOL                            bSTATE = TRUE;
    HMODULE                         hNTDLL = NULL;
    NTSTATUS                        STATUS = NULL;
    HANDLE                          hGetProcessHeap = NULL;
    ULONG                           uReturnLen1 = 0;
    ULONG                           uReturnLen2 = 0;
    PSYSTEM_PROCESS_INFORMATION     pSystemProcInfo = NULL;

    hNTDLL = GetModuleHandleW(L"ntdll.dll");
    if (!hNTDLL) {
        errorWin32("GetModuleHandleW - Failed to get handle to ntdll.dll");
        bSTATE = FALSE;
        goto _cleanUp;
    }
    info_t("GetModuleHandleW - Received handle to ntdll.dll 0x%p", hNTDLL);

    fnNtQuerySystemInformation pNtQuerySystemInformation =
        (fnNtQuerySystemInformation)GetProcAddress(hNTDLL, "NtQuerySystemInformation");
    if (!pNtQuerySystemInformation) {
        errorWin32("GetProcAddress - Failed to address of NtQuerySystemInformation");
        bSTATE = FALSE;
        goto _cleanUp;
    }
    info_t("GetProcAddress - Received address to NtQuerySystemInformation 0x%p", pNtQuerySystemInformation);

    pNtQuerySystemInformation(SystemProcessInformation, NULL, 0, &uReturnLen1);
    info_t("NtQuerySystemInformation - Retrieved size in bytes for the system information: %lu", uReturnLen1);

    hGetProcessHeap = GetProcessHeap();
    pSystemProcInfo = (PSYSTEM_PROCESS_INFORMATION)HeapAlloc(hGetProcessHeap, HEAP_ZERO_MEMORY, (SIZE_T)uReturnLen1);
    if (pSystemProcInfo == NULL) {
        errorWin32("HeapAlloc - failed to allocate memory");
        bSTATE = FALSE;
        goto _cleanUp;
    }
    info_t("HeapAlloc - Allocated %lu bytes of memory for SystemProcessInformation at 0x%p", uReturnLen1, pSystemProcInfo);

    STATUS = pNtQuerySystemInformation(SystemProcessInformation, pSystemProcInfo, uReturnLen1, &uReturnLen2);
    if (STATUS != 0x0) {
        error("NtQuerySystemInformation - failed with error: 0x%0.8X", STATUS);
        HeapFree(hGetProcessHeap, 0, pSystemProcInfo);
        bSTATE = FALSE;
        goto _cleanUp;
    }
    info_t("NtQuerySystemInformation - Retrieved size %lu bytes of system process information at 0x%p", uReturnLen2, pSystemProcInfo);

    *ppSystemProcInfo = pSystemProcInfo;

_cleanUp:
    return bSTATE;
}
