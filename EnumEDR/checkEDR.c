#include "common.h"

static void JsonEscapeW(FILE* out, LPCWSTR str) {
    if (!str) { fputc('"', out); fputc('"', out); return; }
    fputc('"', out);
    for (LPCWSTR p = str; *p; p++) {
        if (*p == L'"' || *p == L'\\')
            fputc('\\', out);
        if (*p < 128)
            fputc((char)*p, out);
        else
            fprintf(out, "\\u%04X", (unsigned)*p);
    }
    fputc('"', out);
}

static void PrintResultsJson(IN const EDR_RESULTS* pResults) {
    printf("{\n  \"detections\": [\n");
    for (DWORD i = 0; i < pResults->count; i++) {
        const EDR_HIT* hit = &pResults->hits[i];
        printf("    {\n");
        printf("      \"vendor\": "); JsonEscapeW(stdout, hit->pwszVendor); printf(",\n");
        printf("      \"type\": \"%s\",\n", hit->type == EDR_COMPONENT_PROCESS ? "process" : "driver");
        printf("      \"component\": "); JsonEscapeW(stdout, hit->pwszComponent); printf(",\n");
        printf("      \"pid\": %lu\n", hit->pid);
        printf("    }%s\n", (i + 1 < pResults->count) ? "," : "");
    }
    printf("  ],\n  \"total\": %lu,\n", pResults->count);

    DWORD vendors = 0, procs = 0, drivers = 0;
    for (DWORD i = 0; i < pResults->count; i++) {
        if (pResults->hits[i].type == EDR_COMPONENT_PROCESS) procs++;
        else drivers++;
    }
    for (DWORD i = 0; i < pResults->count; i++) {
        BOOL dup = FALSE;
        for (DWORD j = 0; j < i; j++) {
            if (wcscmp(pResults->hits[i].pwszVendor, pResults->hits[j].pwszVendor) == 0) {
                dup = TRUE; break;
            }
        }
        if (!dup) vendors++;
    }
    printf("  \"summary\": { \"vendors\": %lu, \"processes\": %lu, \"drivers\": %lu }\n", vendors, procs, drivers);
    printf("}\n");
}

static void PrintResultsTable(IN const EDR_RESULTS* pResults) {
    if (pResults->count == 0) {
        ConsoleWarn("No EDR processes or drivers identified on this system");
        return;
    }

    ConsoleSection("EDR Detections", "matched processes and kernel modules");
    ConsoleTableHeader3(L"Vendor", L"Type", L"Component");

    for (DWORD i = 0; i < pResults->count; i++) {
        const EDR_HIT* hit = &pResults->hits[i];
        int color = ConsoleVendorColor(hit->pwszVendor);
        WCHAR typeLabel[16];
        WCHAR component[MAX_PATH + 32];

        wcscpy_s(typeLabel, _countof(typeLabel),
            hit->type == EDR_COMPONENT_PROCESS ? L"Process" : L"Driver");

        if (hit->type == EDR_COMPONENT_PROCESS)
            swprintf_s(component, _countof(component), L"%s (PID %lu)", hit->pwszComponent, hit->pid);
        else
            swprintf_s(component, _countof(component), L"%s", hit->pwszComponent);

        ConsoleTableRow3Colored(color, hit->pwszVendor, typeLabel, component);
    }
    ConsoleTableFooter();
}

static void PrintResultsSummary(IN const EDR_RESULTS* pResults) {
    if (g_Output.mode == OUTPUT_JSON || g_Output.mode == OUTPUT_QUIET) return;
    if (pResults->count == 0) return;

    DWORD procs = 0, drivers = 0;
    WCHAR vendors[MAX_EDR_HITS][64];
    DWORD vendorCount = 0;

    for (DWORD i = 0; i < pResults->count; i++) {
        if (pResults->hits[i].type == EDR_COMPONENT_PROCESS) procs++;
        else drivers++;

        BOOL known = FALSE;
        for (DWORD v = 0; v < vendorCount; v++) {
            if (wcscmp(vendors[v], pResults->hits[i].pwszVendor) == 0) {
                known = TRUE; break;
            }
        }
        if (!known && vendorCount < MAX_EDR_HITS) {
            wcsncpy_s(vendors[vendorCount], _countof(vendors[vendorCount]),
                pResults->hits[i].pwszVendor, _TRUNCATE);
            vendorCount++;
        }
    }

    char buf[64];
    ConsoleSummaryBegin();
    snprintf(buf, sizeof(buf), "%lu", vendorCount);
    ConsoleSummaryCard("Vendors detected", buf, 5);
    snprintf(buf, sizeof(buf), "%lu", procs);
    ConsoleSummaryCard("Process matches", buf, 1);
    snprintf(buf, sizeof(buf), "%lu", drivers);
    ConsoleSummaryCard("Driver matches", buf, 3);
    snprintf(buf, sizeof(buf), "%lu", pResults->count);
    ConsoleSummaryCard("Total components", buf, 2);

    if (vendorCount > 0) {
        ConsoleBoxLine("");
        ConsoleBoxLine("Active vendors:");
        for (DWORD v = 0; v < vendorCount; v++)
            ConsoleVendorBullet(vendors[v]);
    }
    ConsoleSummaryEnd();
}

static BOOL AddHit(IN OUT EDR_RESULTS* pResults, LPCWSTR vendor, EDR_COMPONENT_TYPE type,
    LPCWSTR component, ULONG pid) {
    if (pResults->count >= MAX_EDR_HITS) return FALSE;
    EDR_HIT* hit = &pResults->hits[pResults->count++];
    hit->pwszVendor = vendor;
    hit->type = type;
    hit->pwszComponent = component;
    hit->pid = pid;
    return TRUE;
}

BOOL DetectEDRs(IN PSYSTEM_MODULE_INFORMATION pSystemModuleInfo,
    IN PSYSTEM_PROCESS_INFORMATION pSystemProcInfo) {

    EDR_RESULTS results = { 0 };
    PSYSTEM_PROCESS_INFORMATION pProcCursor = pSystemProcInfo;

    while (TRUE) {
        if (pProcCursor->ImageName.Buffer != NULL) {
            for (SIZE_T i = 0; i < g_EDRCount; i++) {
                for (DWORD j = 0; g_EDRMap[i].pwszProcessNames[j] != NULL; j++) {
                    if (_wcsicmp(pProcCursor->ImageName.Buffer, g_EDRMap[i].pwszProcessNames[j]) == 0) {
                        AddHit(&results, g_EDRMap[i].pwszEDRName, EDR_COMPONENT_PROCESS,
                            pProcCursor->ImageName.Buffer, (ULONG)(ULONG_PTR)pProcCursor->UniqueProcessId);
                    }
                }
            }
        }
        if (!pProcCursor->NextEntryOffset) break;
        pProcCursor = (PSYSTEM_PROCESS_INFORMATION)((ULONG_PTR)pProcCursor + pProcCursor->NextEntryOffset);
    }

    for (ULONG j = 0; j < pSystemModuleInfo->ModulesCount; j++) {
        CHAR* pszBaseName = strrchr((CHAR*)pSystemModuleInfo->Modules[j].FullPathName, '\\');
        if (pszBaseName != NULL)
            pszBaseName++;
        else
            pszBaseName = (CHAR*)pSystemModuleInfo->Modules[j].FullPathName;

        for (SIZE_T i = 0; i < g_EDRCount; i++) {
            for (DWORD k = 0; g_EDRMap[i].pwszDriverNames[k] != NULL; k++) {
                CHAR szDriverName[MAX_PATH] = { 0 };
                WideCharToMultiByte(CP_ACP, 0, g_EDRMap[i].pwszDriverNames[k], -1,
                    szDriverName, MAX_PATH, NULL, NULL);
                if (_stricmp(szDriverName, pszBaseName) == 0) {
                    static WCHAR driverComponents[MAX_EDR_HITS][MAX_PATH];
                    if (results.count < MAX_EDR_HITS) {
                        MultiByteToWideChar(CP_ACP, 0, pszBaseName, -1,
                            driverComponents[results.count], MAX_PATH);
                        AddHit(&results, g_EDRMap[i].pwszEDRName, EDR_COMPONENT_DRIVER,
                            driverComponents[results.count], 0);
                    }
                }
            }
        }
    }

    if (g_Output.mode == OUTPUT_JSON)
        PrintResultsJson(&results);
    else {
        PrintResultsTable(&results);
        PrintResultsSummary(&results);
    }

    return results.count > 0;
}
