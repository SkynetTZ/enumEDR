#include "common.h"

BOOL PrintDrivers(IN PSYSTEM_MODULE_INFORMATION pSystemModuleInfo) {

    CHAR lpSystemRoot[MAX_PATH];
    CHAR lpResolvedPath[MAX_PATH];

    if (!GetSystemWindowsDirectoryA(lpSystemRoot, MAX_PATH)) {
        error("GetSystemWindowsDirectoryA - Failed, using C:\\Windows");
        strcpy_s(lpSystemRoot, MAX_PATH, "C:\\Windows");
    }

    if (g_Output.mode == OUTPUT_JSON) {
        printf("{\n  \"drivers\": [\n");
        for (ULONG i = 0; i < pSystemModuleInfo->ModulesCount; i++) {
            PSYSTEM_MODULE pModule = &pSystemModuleInfo->Modules[i];
            if (_strnicmp(pModule->FullPathName, "\\SystemRoot\\", 12) == 0)
                snprintf(lpResolvedPath, sizeof(lpResolvedPath), "%s\\%s", lpSystemRoot, pModule->FullPathName + 12);
            else if (_strnicmp(pModule->FullPathName, "\\??\\C:\\WINDOWS\\", 15) == 0)
                snprintf(lpResolvedPath, sizeof(lpResolvedPath), "%s\\%s", lpSystemRoot, pModule->FullPathName + 12);
            else
                snprintf(lpResolvedPath, sizeof(lpResolvedPath), "%s", pModule->FullPathName);

            printf("    \"%s\"%s\n", lpResolvedPath, (i + 1 < pSystemModuleInfo->ModulesCount) ? "," : "");
        }
        printf("  ]\n}\n");
        return TRUE;
    }

    ConsoleSection("Loaded Drivers", "kernel-mode module inventory");
    ConsoleBoxTop("Driver Paths");

    for (ULONG i = 0; i < pSystemModuleInfo->ModulesCount; i++) {
        PSYSTEM_MODULE pModule = &pSystemModuleInfo->Modules[i];

        if (_strnicmp(pModule->FullPathName, "\\SystemRoot\\", 12) == 0)
            snprintf(lpResolvedPath, sizeof(lpResolvedPath), "%s\\%s", lpSystemRoot, pModule->FullPathName + 12);
        else if (_strnicmp(pModule->FullPathName, "\\??\\C:\\WINDOWS\\", 15) == 0)
            snprintf(lpResolvedPath, sizeof(lpResolvedPath), "%s\\%s", lpSystemRoot, pModule->FullPathName + 12);
        else
            snprintf(lpResolvedPath, sizeof(lpResolvedPath), "%s", pModule->FullPathName);

        ConsoleBoxLine("%s", lpResolvedPath);
    }

    ConsoleBoxBottom();
    return TRUE;
}

BOOL EnumerateDrivers(OUT PSYSTEM_MODULE_INFORMATION* ppSystemModuleInfo) {

    BOOL                        bSTATE = TRUE;
    HMODULE                     hNTDLL = NULL;
    NTSTATUS                    STATUS = NULL;
    HANDLE                      hGetProcessHeap = NULL;
    ULONG                       uReturnLen1 = 0;
    ULONG                       uReturnLen2 = 0;
    PSYSTEM_MODULE_INFORMATION  pSystemModuleInfo = NULL;

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

    pNtQuerySystemInformation(SystemModuleInformation, NULL, 0, &uReturnLen1);
    info_t("NtQuerySystemInformation - Retrieved size in bytes for the SystemModuleInformation: %lu", uReturnLen1);

    hGetProcessHeap = GetProcessHeap();
    pSystemModuleInfo = (PSYSTEM_MODULE_INFORMATION)HeapAlloc(hGetProcessHeap, HEAP_ZERO_MEMORY, (SIZE_T)uReturnLen1);
    if (pSystemModuleInfo == NULL) {
        errorWin32("HeapAlloc - failed to allocate memory");
        bSTATE = FALSE;
        goto _cleanUp;
    }
    info_t("HeapAlloc - Allocated %lu bytes of memory for SystemModuleInformation at 0x%p", uReturnLen1, pSystemModuleInfo);

    STATUS = pNtQuerySystemInformation(SystemModuleInformation, pSystemModuleInfo, uReturnLen1, &uReturnLen2);
    if (STATUS != 0x0) {
        error("NtQuerySystemInformation - failed with error: 0x%0.8X", STATUS);
        bSTATE = FALSE;
        goto _cleanUp;
    }
    info_t("NtQuerySystemInformation - Retrieved %lu bytes of SystemModuleInformation at 0x%p", uReturnLen2, pSystemModuleInfo);

    *ppSystemModuleInfo = pSystemModuleInfo;

_cleanUp:
    return bSTATE;
}
