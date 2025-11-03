#include "monitor.h"

#include <stdio.h>

#define CONFIG_LINE_MAX 512

static wchar_t *TrimLeft(wchar_t *text)
{
    while (*text == L' ' || *text == L'\t' || *text == L'\r' || *text == L'\n') {
        ++text;
    }
    return text;
}

static void TrimRight(wchar_t *text)
{
    size_t length = wcslen(text);

    while (length > 0U) {
        wchar_t ch = text[length - 1U];
        if (ch != L' ' && ch != L'\t' && ch != L'\r' && ch != L'\n') {
            break;
        }
        text[length - 1U] = L'\0';
        --length;
    }
}

static void StripQuotes(wchar_t *text)
{
    size_t length = wcslen(text);

    if (length >= 2U && text[0] == L'"' && text[length - 1U] == L'"') {
        MoveMemory(text, text + 1, (length - 2U) * sizeof(wchar_t));
        text[length - 2U] = L'\0';
    }
}

static BOOL SetError(ConfigError *error, DWORD line_number, LPCWSTR message)
{
    if (error == NULL) {
        return FALSE;
    }

    error->line_number = line_number;
    (void)StringCchCopyW(error->message, MRM_MAX_MESSAGE, message);
    return FALSE;
}

static BOOL SplitKeyValue(wchar_t *line, wchar_t **key, wchar_t **value)
{
    wchar_t *colon = wcschr(line, L':');

    if (colon == NULL || key == NULL || value == NULL) {
        return FALSE;
    }

    *colon = L'\0';
    *key = TrimLeft(line);
    TrimRight(*key);

    *value = TrimLeft(colon + 1);
    TrimRight(*value);
    StripQuotes(*value);
    return TRUE;
}

static BOOL IsEmptyString(const wchar_t *text)
{
    return text == NULL || text[0] == L'\0';
}

static BOOL CopyField(wchar_t *target,
                      size_t target_count,
                      LPCWSTR value,
                      ConfigError *error,
                      DWORD line_number,
                      LPCWSTR error_message)
{
    HRESULT hr = StringCchCopyW(target, target_count, value);

    if (FAILED(hr)) {
        return SetError(error, line_number, error_message);
    }

    return TRUE;
}

static BOOL ParseCheckField(MonitorCheck *check,
                            LPCWSTR key,
                            LPCWSTR value,
                            ConfigError *error,
                            DWORD line_number)
{
    if (wcscmp(key, L"name") == 0) {
        if (IsEmptyString(check->name)) {
            return CopyField(check->name, MRM_MAX_NAME, value, error, line_number, L"Check name is too long.");
        }

        return CopyField(check->process_name, MAX_PATH, value, error, line_number, L"Process name is too long.");
    }

    if (wcscmp(key, L"type") == 0) {
        return CopyField(check->type, MRM_MAX_TYPE, value, error, line_number, L"Check type is too long.");
    }

    if (wcscmp(key, L"path") == 0) {
        return CopyField(check->path, MAX_PATH, value, error, line_number, L"Check path is too long.");
    }

    if (wcscmp(key, L"process_name") == 0) {
        return CopyField(check->process_name, MAX_PATH, value, error, line_number, L"Process name is too long.");
    }

    if (wcscmp(key, L"min_free_gb") == 0) {
        wchar_t *end = NULL;
        double parsed = wcstod(value, &end);
        if (end == value || *end != L'\0' || parsed < 0.0) {
            return SetError(error, line_number, L"min_free_gb must be a non-negative number.");
        }
        check->min_free_gb = parsed;
        return TRUE;
    }

    return TRUE;
}

static BOOL IsKnownCheckType(LPCWSTR type)
{
    return wcscmp(type, L"file_exists") == 0 ||
           wcscmp(type, L"disk") == 0 ||
           wcscmp(type, L"process") == 0 ||
           wcscmp(type, L"system") == 0;
}

static BOOL ValidateCheck(const MonitorCheck *check, ConfigError *error)
{
    if (check == NULL) {
        return SetError(error, 0U, L"Internal check validation error.");
    }

    if (IsEmptyString(check->name)) {
        return SetError(error, check->line_number, L"Each check must have a name.");
    }

    if (IsEmptyString(check->type)) {
        return SetError(error, check->line_number, L"Each check must have a type.");
    }

    if (!IsKnownCheckType(check->type)) {
        return SetError(error, check->line_number, L"Unknown check type.");
    }

    if ((wcscmp(check->type, L"file_exists") == 0 || wcscmp(check->type, L"disk") == 0) &&
        IsEmptyString(check->path)) {
        return SetError(error, check->line_number, L"This check type requires path.");
    }

    if (wcscmp(check->type, L"process") == 0 && IsEmptyString(check->process_name)) {
        return SetError(error, check->line_number, L"Process check requires process_name.");
    }

    return TRUE;
}

void MonitorConfigInit(MonitorConfig *config)
{
    if (config == NULL) {
        return;
    }

    SecureZeroMemory(config, sizeof(*config));
    config->interval_seconds = MRM_DEFAULT_INTERVAL_SECONDS;
    (void)StringCchCopyW(config->log_file, MAX_PATH, L"logs/monitor.log");
}

BOOL LoadMonitorConfig(LPCWSTR path, MonitorConfig *config, ConfigError *error)
{
    FILE *file = NULL;
    errno_t open_result = 0;
    wchar_t line[CONFIG_LINE_MAX];
    DWORD line_number = 0U;
    BOOL in_app = FALSE;
    BOOL in_checks = FALSE;
    MonitorCheck *current_check = NULL;

    if (path == NULL || config == NULL) {
        return SetError(error, 0U, L"Invalid configuration loader arguments.");
    }

    MonitorConfigInit(config);

    open_result = _wfopen_s(&file, path, L"r, ccs=UTF-8");
    if (open_result != 0 || file == NULL) {
        return SetError(error, 0U, L"Could not open configuration file.");
    }

    while (fgetws(line, CONFIG_LINE_MAX, file) != NULL) {
        wchar_t *text = NULL;
        wchar_t *key = NULL;
        wchar_t *value = NULL;

        ++line_number;
        text = TrimLeft(line);
        TrimRight(text);

        if (text[0] == L'\0' || text[0] == L'#') {
            continue;
        }

        if (wcscmp(text, L"app:") == 0) {
            in_app = TRUE;
            in_checks = FALSE;
            continue;
        }

        if (wcscmp(text, L"checks:") == 0) {
            in_app = FALSE;
            in_checks = TRUE;
            continue;
        }

        if (in_checks && text[0] == L'-') {
            wchar_t *item_text = TrimLeft(text + 1);

            if (config->check_count >= MRM_MAX_CHECKS) {
                (void)fclose(file);
                return SetError(error, line_number, L"Too many checks configured.");
            }

            current_check = &config->checks[config->check_count];
            SecureZeroMemory(current_check, sizeof(*current_check));
            current_check->line_number = line_number;
            ++config->check_count;

            if (item_text[0] == L'\0') {
                continue;
            }

            if (!SplitKeyValue(item_text, &key, &value)) {
                (void)fclose(file);
                return SetError(error, line_number, L"Expected key: value after check list marker.");
            }

            if (!ParseCheckField(current_check, key, value, error, line_number)) {
                (void)fclose(file);
                return FALSE;
            }
            continue;
        }

        if (in_checks) {
            if (current_check == NULL) {
                (void)fclose(file);
                return SetError(error, line_number, L"Check field appears before a check item.");
            }

            if (!SplitKeyValue(text, &key, &value)) {
                (void)fclose(file);
                return SetError(error, line_number, L"Expected key: value in checks section.");
            }

            if (!ParseCheckField(current_check, key, value, error, line_number)) {
                (void)fclose(file);
                return FALSE;
            }
            continue;
        }

        if (!in_app) {
            continue;
        }

        if (!SplitKeyValue(text, &key, &value)) {
            (void)fclose(file);
            return SetError(error, line_number, L"Expected key: value in app section.");
        }

        if (wcscmp(key, L"interval_seconds") == 0) {
            wchar_t *end = NULL;
            unsigned long parsed = wcstoul(value, &end, 10);
            if (end == value || *end != L'\0' || parsed == 0UL || parsed > 86400UL) {
                (void)fclose(file);
                return SetError(error, line_number, L"interval_seconds must be between 1 and 86400.");
            }
            config->interval_seconds = (DWORD)parsed;
        } else if (wcscmp(key, L"log_file") == 0) {
            HRESULT hr = StringCchCopyW(config->log_file, MAX_PATH, value);
            if (FAILED(hr)) {
                (void)fclose(file);
                return SetError(error, line_number, L"log_file path is too long.");
            }
        }
    }

    (void)fclose(file);

    if (config->check_count == 0U) {
        return SetError(error, 0U, L"Configuration must contain at least one check.");
    }

    for (line_number = 0U; line_number < config->check_count; ++line_number) {
        if (!ValidateCheck(&config->checks[line_number], error)) {
            return FALSE;
        }
    }

    return TRUE;
}
