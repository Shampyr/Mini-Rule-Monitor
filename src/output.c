#include "monitor.h"

static WORD StatusColor(MonitorStatus status, WORD fallback)
{
    switch (status) {
    case MONITOR_STATUS_OK:
        return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    case MONITOR_STATUS_WARN:
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    case MONITOR_STATUS_FAIL:
        return FOREGROUND_RED | FOREGROUND_INTENSITY;
    case MONITOR_STATUS_SKIP:
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    default:
        return fallback;
    }
}

void PrintCheckResult(const CheckResult *result)
{
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    BOOL has_console = FALSE;
    WORD original_attributes = 0U;

    if (result == NULL) {
        return;
    }

    SecureZeroMemory(&console_info, sizeof(console_info));

    if (output != NULL && output != INVALID_HANDLE_VALUE) {
        has_console = GetConsoleScreenBufferInfo(output, &console_info);
        if (has_console) {
            original_attributes = console_info.wAttributes;
            (void)SetConsoleTextAttribute(output, StatusColor(result->status, original_attributes));
        }
    }

    wprintf(L"[%ls]", MonitorStatusText(result->status));

    if (has_console) {
        (void)SetConsoleTextAttribute(output, original_attributes);
    }

    wprintf(L" %ls: %ls\n", result->name, result->message);
}
