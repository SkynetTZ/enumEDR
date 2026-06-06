#pragma once

#include "windows.h"
#include "stdio.h"

#include "structs.h"
#include "typedef.h"
#include "config.h"
#include "console.h"

extern          EDR_MAP g_EDRMap[];
extern const    SIZE_T  g_EDRCount;

#define okay(msg, ...)  ConsoleOk(msg, ##__VA_ARGS__)
#define info(msg, ...)  ConsoleInfo(msg, ##__VA_ARGS__)
#define warn(msg, ...)  ConsoleWarn(msg, ##__VA_ARGS__)
#define error(msg, ...) ConsoleError(msg, ##__VA_ARGS__)
#define debug(msg, ...) ConsoleDebug(msg, ##__VA_ARGS__)

#define okayW(msg, ...)  ConsoleOkW(msg, ##__VA_ARGS__)
#define infoW(msg, ...)  ConsoleInfoW(msg, ##__VA_ARGS__)
#define warnW(msg, ...)  ConsoleWarnW(msg, ##__VA_ARGS__)
#define errorW(msg, ...) ConsoleErrorW(msg, ##__VA_ARGS__)

#define NT_SUCCESS(status) (((NTSTATUS)(status)) >= 0)

#define infoW_t(msg, ...) do { if (g_Output.mode == OUTPUT_VERBOSE) ConsoleInfoW(msg, ##__VA_ARGS__); } while(0)
#define info_t(msg, ...)  do { if (g_Output.mode == OUTPUT_VERBOSE) ConsoleDebug(msg, ##__VA_ARGS__); } while(0)

#define NtCurrentProcess() ((HANDLE)-1)
#define NtCurrentThread()  ((HANDLE)-2)

int errorWin32(IN const char* msg);
int errorNT(IN const char* msg, IN NTSTATUS ntstatus);
void print_bytes(IN void* ptr, IN int size);

BOOL EnumerateProcesses(OUT PSYSTEM_PROCESS_INFORMATION* ppSystemProcInfo);
BOOL PrintProcesses(IN PSYSTEM_PROCESS_INFORMATION pSystemProcInfo);
DWORD CountProcesses(IN PSYSTEM_PROCESS_INFORMATION pSystemProcInfo);

BOOL EnumerateDrivers(OUT PSYSTEM_MODULE_INFORMATION* ppSystemModuleInfo);
BOOL PrintDrivers(IN PSYSTEM_MODULE_INFORMATION pSystemModuleInfo);

BOOL DetectEDRs(IN PSYSTEM_MODULE_INFORMATION pSystemModuleInfo, IN PSYSTEM_PROCESS_INFORMATION pSystemProcInfo);
