#pragma once

#include <windows.h>

typedef enum _OUTPUT_MODE {
    OUTPUT_NORMAL = 0,
    OUTPUT_QUIET,
    OUTPUT_VERBOSE,
    OUTPUT_JSON
} OUTPUT_MODE;

typedef struct _OUTPUT_CONFIG {
    OUTPUT_MODE mode;
    BOOL        useColor;
    BOOL        showBanner;
    BOOL        showSteps;
} OUTPUT_CONFIG;

extern OUTPUT_CONFIG g_Output;

// Lifecycle
BOOL ConsoleInit(void);
void ConsoleShutdown(void);

// Styled logging (replaces raw printf macros)
void ConsoleOk(const char* fmt, ...);
void ConsoleInfo(const char* fmt, ...);
void ConsoleWarn(const char* fmt, ...);
void ConsoleError(const char* fmt, ...);
void ConsoleDebug(const char* fmt, ...);

void ConsoleOkW(const wchar_t* fmt, ...);
void ConsoleInfoW(const wchar_t* fmt, ...);
void ConsoleWarnW(const wchar_t* fmt, ...);
void ConsoleErrorW(const wchar_t* fmt, ...);

// Layout primitives
void ConsolePrintBanner(void);
void ConsolePrintHelp(const char* fileName);
void ConsoleSection(const char* title, const char* subtitle);
void ConsoleDivider(void);
void ConsoleNewline(void);

void ConsoleBoxTop(const char* title);
void ConsoleBoxLine(const char* fmt, ...);
void ConsoleBoxLineW(const wchar_t* fmt, ...);
void ConsoleBoxBottom(void);

void ConsoleTableHeader3(const wchar_t* c1, const wchar_t* c2, const wchar_t* c3);
void ConsoleTableHeader2(const wchar_t* c1, const wchar_t* c2);
void ConsoleTableRow3Colored(int colorId, const wchar_t* c1, const wchar_t* c2, const wchar_t* c3);
void ConsoleTableRow2(const wchar_t* c1, const wchar_t* c2);
void ConsoleTableSeparator(void);
void ConsoleTableFooter(void);

// Summary dashboard
void ConsoleSummaryCard(const char* label, const char* value, int accentColor);
void ConsoleSummaryBegin(void);
void ConsoleSummaryEnd(void);

// Progress step indicator
void ConsoleStepBegin(const char* label);
void ConsoleStepEnd(BOOL success, const char* detailFmt, ...);

// ANSI helpers
const char* ConsoleColor(int colorId);
const char* ConsoleReset(void);
int ConsoleVendorColor(LPCWSTR vendorName);
void ConsoleVendorBullet(LPCWSTR vendorName);
