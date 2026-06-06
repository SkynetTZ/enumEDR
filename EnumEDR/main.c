#include "common.h"

static BOOL IsCommand(const char* arg) {
    return strcmp(arg, "--edr") == 0
        || strcmp(arg, "--processes") == 0
        || strcmp(arg, "--drivers") == 0
        || strcmp(arg, "-h") == 0;
}

static void ParseOptions(int argc, char** argv, int* pCommandIndex) {
    *pCommandIndex = -1;

    for (int i = 1; i < argc; i++) {
        if (IsCommand(argv[i])) {
            *pCommandIndex = i;
            return;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            g_Output.mode = OUTPUT_VERBOSE;
            g_Output.showSteps = TRUE;
        }
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            g_Output.mode = OUTPUT_QUIET;
            g_Output.showBanner = FALSE;
            g_Output.showSteps = FALSE;
        }
        else if (strcmp(argv[i], "--json") == 0) {
            g_Output.mode = OUTPUT_JSON;
            g_Output.showBanner = FALSE;
            g_Output.showSteps = FALSE;
            g_Output.useColor = FALSE;
        }
        else if (strcmp(argv[i], "--no-color") == 0) {
            g_Output.useColor = FALSE;
        }
        else if (strcmp(argv[i], "--no-banner") == 0) {
            g_Output.showBanner = FALSE;
        }
        else if (argv[i][0] == '-') {
            error("Unknown option: %s", argv[i]);
        }
    }
}

int main(int argc, char** argv) {

    PSYSTEM_MODULE_INFORMATION  pSystemModuleInfo = NULL;
    PSYSTEM_PROCESS_INFORMATION pSystemProcInfo = NULL;
    PVOID                       pValueToFree = NULL;
    DWORD                       dwProcessCount = 0;
    int                         commandIndex = -1;
    const char*                 command = NULL;

    ConsoleInit();

    if (argc < 2) {
        ConsolePrintHelp(argv[0]);
        return EXIT_FAILURE;
    }

    ParseOptions(argc, argv, &commandIndex);
    if (commandIndex < 0) {
        ConsolePrintHelp(argv[0]);
        return EXIT_FAILURE;
    }

    command = argv[commandIndex];
    if (strcmp(command, "-h") == 0) {
        ConsolePrintHelp(argv[0]);
        return EXIT_SUCCESS;
    }

    if (g_Output.showBanner)
        ConsolePrintBanner();

    if (strcmp(command, "--processes") == 0) {

        ConsoleStepBegin("Enumerating running processes");
        if (!EnumerateProcesses(&pSystemProcInfo)) {
            ConsoleStepEnd(FALSE, "Failed to enumerate processes");
            return EXIT_FAILURE;
        }
        dwProcessCount = CountProcesses(pSystemProcInfo);
        ConsoleStepEnd(TRUE, "Found %lu processes", dwProcessCount);
        pValueToFree = pSystemProcInfo;

        if (!PrintProcesses(pSystemProcInfo)) {
            error("PrintProcesses - Failed to print");
            return EXIT_FAILURE;
        }
    }
    else if (strcmp(command, "--drivers") == 0) {

        ConsoleStepBegin("Enumerating loaded kernel drivers");
        if (!EnumerateDrivers(&pSystemModuleInfo)) {
            ConsoleStepEnd(FALSE, "Failed to enumerate drivers");
            return EXIT_FAILURE;
        }
        ConsoleStepEnd(TRUE, "Found %lu drivers", pSystemModuleInfo->ModulesCount);

        if (!PrintDrivers(pSystemModuleInfo)) {
            error("PrintDrivers - Failed to print");
            return EXIT_FAILURE;
        }
    }
    else if (strcmp(command, "--edr") == 0) {

        ConsoleStepBegin("Enumerating running processes");
        if (!EnumerateProcesses(&pSystemProcInfo)) {
            ConsoleStepEnd(FALSE, "Failed to enumerate processes");
            return EXIT_FAILURE;
        }
        dwProcessCount = CountProcesses(pSystemProcInfo);
        ConsoleStepEnd(TRUE, "Scanned %lu processes", dwProcessCount);

        ConsoleStepBegin("Enumerating loaded kernel drivers");
        if (!EnumerateDrivers(&pSystemModuleInfo)) {
            ConsoleStepEnd(FALSE, "Failed to enumerate drivers");
            return EXIT_FAILURE;
        }
        ConsoleStepEnd(TRUE, "Scanned %lu drivers", pSystemModuleInfo->ModulesCount);

        pValueToFree = pSystemProcInfo;

        if (g_Output.mode != OUTPUT_JSON)
            ConsoleSection("Threat Surface Analysis", "cross-referencing known EDR signatures");

        if (!DetectEDRs(pSystemModuleInfo, pSystemProcInfo)) {
            if (g_Output.mode != OUTPUT_JSON)
                error("No EDR processes or drivers identified");
            if (pValueToFree) HeapFree(GetProcessHeap(), 0, pValueToFree);
            if (pSystemModuleInfo) HeapFree(GetProcessHeap(), 0, pSystemModuleInfo);
            return EXIT_FAILURE;
        }

        if (g_Output.mode != OUTPUT_JSON) {
            ConsoleNewline();
            okay("EDR enumeration complete");
        }
    }
    else {
        error("Unknown command: %s", command);
        ConsolePrintHelp(argv[0]);
        return EXIT_FAILURE;
    }

    if (pValueToFree)
        HeapFree(GetProcessHeap(), 0, pValueToFree);
    if (pSystemModuleInfo)
        HeapFree(GetProcessHeap(), 0, pSystemModuleInfo);

    ConsoleShutdown();
    return EXIT_SUCCESS;
}
