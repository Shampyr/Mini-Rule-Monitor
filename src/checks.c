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

BOOL RunMonitorCheck(const MonitorCheck *check, CheckResult *result)
{
    if (check == NULL || result == NULL) {
        return FALSE;
    }

    if (wcscmp(check->type, L"file_exists") == 0) {
        return RunFileExistsCheck(check, result);
    }

    return SetResult(result, check, MONITOR_STATUS_SKIP, L"check type is not implemented yet");
}
