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
    HRESULT hr = S_OK;

    if (error == NULL) {
        return FALSE;
    }

    error->line_number = line_number;
    hr = StringCchCopyW(error->message, MRM_MAX_MESSAGE, message);
    return SUCCEEDED(hr) ? FALSE : FALSE;
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
            continue;
        }

        if (wcscmp(text, L"checks:") == 0) {
            in_app = FALSE;
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
    return TRUE;
}
