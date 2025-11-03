#include "monitor.h"

#include <stdio.h>

static BOOL EnsureLogDirectory(LPCWSTR log_file)
{
    wchar_t directory[MAX_PATH];
    wchar_t *last_slash = NULL;

    if (log_file == NULL) {
        return FALSE;
    }

    SecureZeroMemory(directory, sizeof(directory));
    if (FAILED(StringCchCopyW(directory, MAX_PATH, log_file))) {
        return FALSE;
    }

    last_slash = wcsrchr(directory, L'\\');
    if (last_slash == NULL) {
        last_slash = wcsrchr(directory, L'/');
    }

    if (last_slash == NULL) {
        return TRUE;
    }

    *last_slash = L'\0';
    if (directory[0] == L'\0') {
        return TRUE;
    }

    if (CreateDirectoryW(directory, NULL)) {
        return TRUE;
    }

    {
        DWORD error_code = GetLastError();
        return error_code == ERROR_ALREADY_EXISTS;
    }
}

BOOL AppendCheckResultToLog(LPCWSTR log_file, const CheckResult *result)
{
    FILE *file = NULL;
    errno_t open_result = 0;
    SYSTEMTIME now;
    int write_result = 0;
    int close_result = 0;

    if (log_file == NULL || result == NULL) {
        return FALSE;
    }

    if (!EnsureLogDirectory(log_file)) {
        return FALSE;
    }

    SecureZeroMemory(&now, sizeof(now));
    GetLocalTime(&now);

    open_result = _wfopen_s(&file, log_file, L"a, ccs=UTF-8");
    if (open_result != 0 || file == NULL) {
        return FALSE;
    }

    write_result = fwprintf(file,
                            L"%04hu-%02hu-%02hu %02hu:%02hu:%02hu %ls %ls %ls\n",
                            now.wYear,
                            now.wMonth,
                            now.wDay,
                            now.wHour,
                            now.wMinute,
                            now.wSecond,
                            MonitorStatusText(result->status),
                            result->name,
                            result->message);

    close_result = fclose(file);
    return write_result >= 0 && close_result == 0;
}
