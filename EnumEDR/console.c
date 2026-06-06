#include "console.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

OUTPUT_CONFIG g_Output = {
    OUTPUT_NORMAL, TRUE, TRUE, TRUE
};

#define C_RESET   "\x1b[0m"
#define C_BOLD    "\x1b[1m"
#define C_DIM     "\x1b[2m"
#define C_RED     "\x1b[38;5;203m"
#define C_GREEN   "\x1b[38;5;78m"
#define C_YELLOW  "\x1b[38;5;220m"
#define C_BLUE    "\x1b[38;5;39m"
#define C_MAGENTA "\x1b[38;5;141m"
#define C_CYAN    "\x1b[38;5;51m"
#define C_WHITE   "\x1b[38;5;255m"
#define C_GRAY    "\x1b[38;5;245m"
#define C_ORANGE  "\x1b[38;5;208m"
#define C_PURPLE  "\x1b[38;5;99m"
#define C_TEAL    "\x1b[38;5;43m"

static const char* VENDOR_COLORS[] = {
    C_BLUE, C_CYAN, C_YELLOW, C_MAGENTA, C_ORANGE,
    C_RED, C_PURPLE, C_TEAL, C_GREEN, C_WHITE
};
static const int VENDOR_COLOR_COUNT = sizeof(VENDOR_COLORS) / sizeof(VENDOR_COLORS[0]);

static BOOL g_VtEnabled = FALSE;

static BOOL ConsoleIsTty(void) {
    DWORD mode = 0;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return FALSE;
    return GetConsoleMode(hOut, &mode) != 0;
}

static void ConsoleWriteWide(const wchar_t* text) {
    if (!text || !text[0]) return;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (ConsoleIsTty()) {
        DWORD written = 0;
        WriteConsoleW(hOut, text, (DWORD)wcslen(text), &written, NULL);
        return;
    }
    char utf8[2048];
    int len = WideCharToMultiByte(CP_UTF8, 0, text, -1, utf8, (int)sizeof(utf8), NULL, NULL);
    if (len > 0)
        fwrite(utf8, 1, (size_t)len - 1, stdout);
}

static void ConsoleWriteWideLine(const wchar_t* text) {
    ConsoleWriteWide(text);
    fputc('\n', stdout);
}

BOOL ConsoleInit(void) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            g_VtEnabled = SetConsoleMode(hOut, mode) != 0;
        }
    }

    if (!ConsoleIsTty())
        g_Output.useColor = FALSE;

    return TRUE;
}

void ConsoleShutdown(void) {}

const char* ConsoleReset(void) {
    return g_Output.useColor ? C_RESET : "";
}

const char* ConsoleColor(int colorId) {
    if (!g_Output.useColor) return "";
    switch (colorId) {
    case 0:  return C_RED;
    case 1:  return C_GREEN;
    case 2:  return C_YELLOW;
    case 3:  return C_BLUE;
    case 4:  return C_MAGENTA;
    case 5:  return C_CYAN;
    case 6:  return C_ORANGE;
    case 7:  return C_PURPLE;
    case 8:  return C_TEAL;
    case 9:  return C_WHITE;
    default: return C_GRAY;
    }
}

int ConsoleVendorColor(LPCWSTR vendorName) {
    if (!vendorName) return 9;
    unsigned hash = 0;
    for (LPCWSTR p = vendorName; *p; p++)
        hash = hash * 31u + (unsigned)(*p & 0xFF);
    return (int)(hash % VENDOR_COLOR_COUNT);
}

static void VPrintF(FILE* stream, const char* prefix, const char* color, const char* fmt, va_list args) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    if (g_Output.useColor && color && color[0])
        fprintf(stream, "%s%s%s ", color, prefix, C_RESET);
    else
        fprintf(stream, "%s ", prefix);
    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
}

void ConsoleOk(const char* fmt, ...) {
    va_list args; va_start(args, fmt);
    VPrintF(stdout, "[+]", C_GREEN, fmt, args);
    va_end(args);
}

void ConsoleInfo(const char* fmt, ...) {
    va_list args; va_start(args, fmt);
    VPrintF(stdout, "[i]", C_CYAN, fmt, args);
    va_end(args);
}

void ConsoleWarn(const char* fmt, ...) {
    va_list args; va_start(args, fmt);
    VPrintF(stdout, "[!]", C_YELLOW, fmt, args);
    va_end(args);
}

void ConsoleError(const char* fmt, ...) {
    va_list args; va_start(args, fmt);
    VPrintF(stderr, "[-]", C_RED, fmt, args);
    va_end(args);
}

void ConsoleDebug(const char* fmt, ...) {
    if (g_Output.mode != OUTPUT_VERBOSE) return;
    va_list args; va_start(args, fmt);
    VPrintF(stdout, "[.]", C_GRAY, fmt, args);
    va_end(args);
}

static void VWPrintF(FILE* stream, const wchar_t* prefix, const char* color, const wchar_t* fmt, va_list args) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    if (g_Output.useColor && color && color[0])
        fprintf(stream, "%s%ls%s ", color, prefix, C_RESET);
    else
        fprintf(stream, "%ls ", prefix);
    vfwprintf(stream, fmt, args);
    fprintf(stream, "\n");
}

void ConsoleOkW(const wchar_t* fmt, ...) {
    va_list args; va_start(args, fmt);
    VWPrintF(stdout, L"[+]", C_GREEN, fmt, args);
    va_end(args);
}

void ConsoleInfoW(const wchar_t* fmt, ...) {
    va_list args; va_start(args, fmt);
    VWPrintF(stdout, L"[i]", C_CYAN, fmt, args);
    va_end(args);
}

void ConsoleWarnW(const wchar_t* fmt, ...) {
    va_list args; va_start(args, fmt);
    VWPrintF(stdout, L"[!]", C_YELLOW, fmt, args);
    va_end(args);
}

void ConsoleErrorW(const wchar_t* fmt, ...) {
    va_list args; va_start(args, fmt);
    VWPrintF(stderr, L"[-]", C_RED, fmt, args);
    va_end(args);
}

void ConsoleNewline(void) {
    if (g_Output.mode != OUTPUT_QUIET && g_Output.mode != OUTPUT_JSON)
        printf("\n");
}

void ConsoleDivider(void) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    if (g_Output.useColor)
        printf("%s%s%s\n", C_DIM, "  ─────────────────────────────────────────────────────────────────", C_RESET);
    else
        printf("  ----------------------------------------------------------------\n");
}

void ConsolePrintBanner(void) {
    if (!g_Output.showBanner || g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;

    if (g_Output.useColor) {
        printf("\n%s", C_CYAN);
        printf("  ╔══════════════════════════════════════════════════════════════════╗\n");
        printf("  ║%s%s  E N U M   E D R%s%s                                              %s║\n", C_BOLD, C_WHITE, C_CYAN, C_RESET, C_CYAN);
        printf("  ║%s  Endpoint Detection & Response Enumeration Suite%s%s               ║\n", C_DIM, C_CYAN, C_RESET);
        printf("  ╚══════════════════════════════════════════════════════════════════╝\n");
        printf("%s\n", C_RESET);
    } else {
        printf("\n  ================================================================\n");
        printf("    ENUM EDR - Endpoint Detection & Response Enumeration Suite\n");
        printf("  ================================================================\n\n");
    }
}

void ConsoleSection(const char* title, const char* subtitle) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    ConsoleNewline();
    if (g_Output.useColor) {
        printf("  %s%s%s%s\n", C_BOLD, C_MAGENTA, title, C_RESET);
        if (subtitle && subtitle[0])
            printf("  %s%s%s\n", C_DIM, subtitle, C_RESET);
    } else {
        printf("  %s\n", title);
        if (subtitle && subtitle[0])
            printf("  %s\n", subtitle);
    }
    ConsoleDivider();
}

void ConsoleBoxTop(const char* title) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    if (g_Output.useColor)
        printf("  %s┌─ %s%s%s ─────────────────────────────────────────────%s\n", C_DIM, C_BOLD, title, C_DIM, C_RESET);
    else
        printf("  +-- %s ------------------------------------------------\n", title);
}

void ConsoleBoxLine(const char* fmt, ...) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    va_list args;
    printf("  %s│%s  ", g_Output.useColor ? C_DIM : "", g_Output.useColor ? C_RESET : "");
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void ConsoleBoxLineW(const wchar_t* fmt, ...) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    va_list args;
    wprintf(L"  %ls│%ls  ", g_Output.useColor ? L"\x1b[2m" : L"", g_Output.useColor ? L"\x1b[0m" : L"");
    va_start(args, fmt);
    vwprintf(fmt, args);
    va_end(args);
    wprintf(L"\n");
}

void ConsoleBoxBottom(void) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    if (g_Output.useColor)
        printf("  %s└──────────────────────────────────────────────────────────────%s\n", C_DIM, C_RESET);
    else
        printf("  +--------------------------------------------------------------\n");
}

static void PrintBorder3(const char* line) {
    if (g_Output.useColor)
        printf("  %s%s%s\n", C_DIM, line, C_RESET);
    else
        printf("  +----------------------------------+------------+--------------------------------------+\n");
}

void ConsoleTableHeader3(const wchar_t* c1, const wchar_t* c2, const wchar_t* c3) {
    wchar_t row[512];
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    PrintBorder3("+----------------------------------+------------+--------------------------------------+");
    swprintf_s(row, _countof(row), L"  | %-32s | %-10s | %-36s |", c1, c2, c3);
    if (g_Output.useColor) printf("%s", C_BOLD);
    ConsoleWriteWideLine(row);
    if (g_Output.useColor) printf("%s", C_RESET);
    ConsoleTableSeparator();
}

void ConsoleTableHeader2(const wchar_t* c1, const wchar_t* c2) {
    wchar_t row[512];
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    PrintBorder3("+------------------------------------------+----------+");
    swprintf_s(row, _countof(row), L"  | %-38s | %-8s |", c1, c2);
    if (g_Output.useColor) printf("%s", C_BOLD);
    ConsoleWriteWideLine(row);
    if (g_Output.useColor) printf("%s", C_RESET);
    if (g_Output.useColor)
        printf("  %s+------------------------------------------+----------+%s\n", C_DIM, C_RESET);
    else
        printf("  +------------------------------------------+----------+\n");
}

void ConsoleTableSeparator(void) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    PrintBorder3("+----------------------------------+------------+--------------------------------------+");
}

void ConsoleTableRow3Colored(int colorId, const wchar_t* c1, const wchar_t* c2, const wchar_t* c3) {
    wchar_t row[512];
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    swprintf_s(row, _countof(row), L"  | %-32s | %-10s | %-36s |", c1, c2, c3);
    if (g_Output.useColor) printf("%s", VENDOR_COLORS[colorId % VENDOR_COLOR_COUNT]);
    ConsoleWriteWideLine(row);
    if (g_Output.useColor) printf("%s", C_RESET);
}

void ConsoleTableRow2(const wchar_t* c1, const wchar_t* c2) {
    wchar_t row[512];
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    swprintf_s(row, _countof(row), L"  | %-38s | %-8s |", c1, c2);
    ConsoleWriteWideLine(row);
}

void ConsoleTableFooter(void) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    PrintBorder3("+----------------------------------+------------+--------------------------------------+");
}

void ConsoleSummaryBegin(void) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    ConsoleSection("Detection Summary", "aggregated EDR footprint");
    ConsoleBoxTop("Statistics");
}

void ConsoleSummaryCard(const char* label, const char* value, int accentColor) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    if (g_Output.useColor)
        printf("  %s│%s  %-22s %s%s%s\n", C_DIM, C_RESET, label, ConsoleColor(accentColor), value, C_RESET);
    else
        printf("  |  %-22s %s\n", label, value);
}

void ConsoleSummaryEnd(void) {
    ConsoleBoxBottom();
}

void ConsoleStepBegin(const char* label) {
    if (!g_Output.showSteps || g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    if (g_Output.useColor)
        printf("  %s▸%s %s%s%s\n", C_CYAN, C_RESET, C_BOLD, label, C_RESET);
    else
        printf("  > %s\n", label);
}

void ConsoleStepEnd(BOOL success, const char* detailFmt, ...) {
    if (!g_Output.showSteps || g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    va_list args;
    if (g_Output.useColor)
        printf("    %s%s%s ", success ? C_GREEN : C_RED, success ? "✓" : "✗", C_RESET);
    else
        printf("    %s ", success ? "OK" : "FAIL");
    va_start(args, detailFmt);
    vprintf(detailFmt, args);
    va_end(args);
    printf("\n");
}

void ConsoleVendorBullet(LPCWSTR vendorName) {
    if (g_Output.mode == OUTPUT_QUIET || g_Output.mode == OUTPUT_JSON) return;
    const char* vc = VENDOR_COLORS[ConsoleVendorColor(vendorName) % VENDOR_COLOR_COUNT];
    if (g_Output.useColor) {
        printf("  %s│%s    %s•%s ", C_DIM, C_RESET, vc, C_RESET);
        wprintf(L"%ls\n", vendorName);
    } else {
        wprintf(L"  |    - %ls\n", vendorName);
    }
}

void ConsolePrintHelp(const char* fileName) {
    if (g_Output.useColor) {
        printf("\n%s%sUsage:%s %s [options] <command>\n\n", C_BOLD, C_CYAN, C_RESET, fileName);
        printf("%sCommands:%s\n", C_BOLD, C_RESET);
        printf("  %s--edr%s       Detect EDR processes and kernel drivers\n", C_GREEN, C_RESET);
        printf("  %s--processes%s List all running processes\n", C_GREEN, C_RESET);
        printf("  %s--drivers%s   List all loaded kernel drivers\n", C_GREEN, C_RESET);
        printf("  %s-h%s          Show this help message\n\n", C_GREEN, C_RESET);
        printf("%sOptions:%s\n", C_BOLD, C_RESET);
        printf("  %s-v, --verbose%s   Show detailed enumeration steps\n", C_YELLOW, C_RESET);
        printf("  %s-q, --quiet%s     Minimal output (results only)\n", C_YELLOW, C_RESET);
        printf("  %s--json%s          Machine-readable JSON output\n", C_YELLOW, C_RESET);
        printf("  %s--no-color%s      Disable ANSI colors\n", C_YELLOW, C_RESET);
        printf("  %s--no-banner%s     Skip startup banner\n\n", C_YELLOW, C_RESET);
    } else {
        printf("\nUsage: %s [options] <command>\n\n", fileName);
        printf("Commands:\n  --edr  --processes  --drivers  -h\n\n");
        printf("Options:\n  -v --verbose  -q --quiet  --json  --no-color  --no-banner\n\n");
    }
}
