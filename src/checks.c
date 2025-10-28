#include "monitor.h"

static BOOL SetResult(CheckResult *result,
                      const MonitorCheck *check,
                      MonitorStatus status,
                      LPCWSTR message)
{
    HRESULT hr = S_OK;

    if (result == NULL || check == NULL || message == NULL) {
        return FALSE;
    }

    SecureZeroMemory(result, sizeof(*result));
    result->status = status;

    hr = StringCchCopyW(result->name, MRM_MAX_NAME, check->name);
    if (FAILED(hr)) {
        return FALSE;
    }

    hr = StringCchCopyW(result->type, MRM_MAX_TYPE, check->type);
    if (FAILED(hr)) {
        return FALSE;
    }

    hr = StringCchCopyW(result->message, MRM_MAX_MESSAGE, message);
    return SUCCEEDED(hr);
}

static BOOL RunFileExistsCheck(const MonitorCheck *check, CheckResult *result)
{
    DWORD attributes = GetFileAttributesW(check->path);

    if (attributes == INVALID_FILE_ATTRIBUTES) {
        DWORD error_code = GetLastError();

        if (error_code == ERROR_FILE_NOT_FOUND || error_code == ERROR_PATH_NOT_FOUND) {
            return SetResult(result, check, MONITOR_STATUS_FAIL, L"file was not found");
        }

        return SetResult(result, check, MONITOR_STATUS_FAIL, L"could not read file attributes");
    }

    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
        return SetResult(result, check, MONITOR_STATUS_WARN, L"path exists but is a directory");
    }

    return SetResult(result, check, MONITOR_STATUS_OK, L"file exists");
}

static BOOL RunDiskCheck(const MonitorCheck *check, CheckResult *result)
{
    ULARGE_INTEGER available_bytes;
    ULARGE_INTEGER total_bytes;
    wchar_t message[MRM_MAX_MESSAGE];
    double available_gb = 0.0;
    BOOL ok = FALSE;

    SecureZeroMemory(&available_bytes, sizeof(available_bytes));
    SecureZeroMemory(&total_bytes, sizeof(total_bytes));
    SecureZeroMemory(message, sizeof(message));

    /*
     * The last parameter is NULL because the monitor only reports total size
     * and user-available free bytes; total free bytes ignoring quota is unused.
     */
    ok = GetDiskFreeSpaceExW(check->path, &available_bytes, &total_bytes, NULL);
    if (!ok) {
        DWORD error_code = GetLastError();
        HRESULT hr = StringCchPrintfW(message,
                                      MRM_MAX_MESSAGE,
                                      L"could not read disk space, error=%lu",
                                      error_code);
        if (FAILED(hr)) {
            return FALSE;
        }
        return SetResult(result, check, MONITOR_STATUS_FAIL, message);
    }

    available_gb = (double)available_bytes.QuadPart / (1024.0 * 1024.0 * 1024.0);

    if (available_gb < check->min_free_gb) {
        HRESULT hr = StringCchPrintfW(message,
                                      MRM_MAX_MESSAGE,
                                      L"free_gb=%.2f required_gb=%.2f",
                                      available_gb,
                                      check->min_free_gb);
        if (FAILED(hr)) {
            return FALSE;
        }
        return SetResult(result, check, MONITOR_STATUS_WARN, message);
    }

    {
        double total_gb = (double)total_bytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
        HRESULT hr = StringCchPrintfW(message,
                                      MRM_MAX_MESSAGE,
                                      L"free_gb=%.2f total_gb=%.2f",
                                      available_gb,
                                      total_gb);
        if (FAILED(hr)) {
            return FALSE;
        }
    }

    return SetResult(result, check, MONITOR_STATUS_OK, message);
}

BOOL RunMonitorCheck(const MonitorCheck *check, CheckResult *result)
{
    if (check == NULL || result == NULL) {
        return FALSE;
    }

    if (wcscmp(check->type, L"file_exists") == 0) {
        return RunFileExistsCheck(check, result);
    }

    if (wcscmp(check->type, L"disk") == 0) {
        return RunDiskCheck(check, result);
    }

    return SetResult(result, check, MONITOR_STATUS_SKIP, L"check type is not implemented yet");
}
