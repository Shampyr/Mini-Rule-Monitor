#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include "monitor.h"

static void PrintUsage(void)
{
    wprintf(L"Usage:\n");
    wprintf(L"  mini-rule-monitor.exe validate-config [--config config.yaml]\n");
    wprintf(L"  mini-rule-monitor.exe list-checks [--config config.yaml]\n");
    wprintf(L"  mini-rule-monitor.exe check-once [--config config.yaml]\n");
    wprintf(L"  mini-rule-monitor.exe run [--config config.yaml]\n");
}

static BOOL ParseCommand(int argc,
                         wchar_t **argv,
                         MonitorCommand *command,
                         LPCWSTR *config_path)
{
    int index = 0;

    if (command == NULL || config_path == NULL) {
        return FALSE;
    }

    *command = MONITOR_COMMAND_HELP;
    *config_path = MRM_DEFAULT_CONFIG_PATH;

    if (argc < 2) {
        return TRUE;
    }

    if (wcscmp(argv[1], L"validate-config") == 0) {
        *command = MONITOR_COMMAND_VALIDATE_CONFIG;
    } else if (wcscmp(argv[1], L"list-checks") == 0) {
        *command = MONITOR_COMMAND_LIST_CHECKS;
    } else if (wcscmp(argv[1], L"check-once") == 0) {
        *command = MONITOR_COMMAND_CHECK_ONCE;
    } else if (wcscmp(argv[1], L"run") == 0) {
        *command = MONITOR_COMMAND_RUN;
    } else {
        return FALSE;
    }

    index = 2;
    while (index < argc) {
        if (wcscmp(argv[index], L"--config") == 0) {
            if ((index + 1) >= argc) {
                return FALSE;
            }
            *config_path = argv[index + 1];
            index += 2;
            continue;
        }

        return FALSE;
    }

    return TRUE;
}

static void RunAllChecks(const MonitorConfig *config)
{
    DWORD index = 0U;

    if (config == NULL) {
        return;
    }

    for (index = 0U; index < config->check_count; ++index) {
        CheckResult result;
        SecureZeroMemory(&result, sizeof(result));

        if (!RunMonitorCheck(&config->checks[index], &result)) {
            wprintf(L"[FAIL] %ls: internal check error\n", config->checks[index].name);
            continue;
        }

        PrintCheckResult(&result);
        if (!AppendCheckResultToLog(config->log_file, &result)) {
            wprintf(L"[WARN] could not append to log file: %ls\n", config->log_file);
        }
    }
}

int wmain(int argc, wchar_t **argv)
{
    MonitorCommand command = MONITOR_COMMAND_HELP;
    LPCWSTR config_path = MRM_DEFAULT_CONFIG_PATH;
    MonitorConfig config;
    ConfigError error;

    if (!ParseCommand(argc, argv, &command, &config_path)) {
        wprintf(L"Invalid command line.\n\n");
        PrintUsage();
        return 2;
    }

    if (command == MONITOR_COMMAND_HELP) {
        PrintUsage();
        return 0;
    }

    wprintf(L"Mini Rule Monitor\n");
    SecureZeroMemory(&config, sizeof(config));
    SecureZeroMemory(&error, sizeof(error));

    if (!LoadMonitorConfig(config_path, &config, &error)) {
        if (error.line_number > 0U) {
            wprintf(L"Config error on line %lu: %ls\n", error.line_number, error.message);
        } else {
            wprintf(L"Config error: %ls\n", error.message);
        }
        return 1;
    }

    if (command == MONITOR_COMMAND_VALIDATE_CONFIG) {
        wprintf(L"Config OK: %ls\n", config_path);
        wprintf(L"Interval: %lu seconds\n", config.interval_seconds);
        wprintf(L"Log file: %ls\n", config.log_file);
        wprintf(L"Checks: %lu\n", config.check_count);
        return 0;
    }

    if (command == MONITOR_COMMAND_LIST_CHECKS) {
        DWORD index = 0U;

        for (index = 0U; index < config.check_count; ++index) {
            wprintf(L"%lu. %ls [%ls]\n",
                    index + 1U,
                    config.checks[index].name,
                    config.checks[index].type);
        }
        return 0;
    }

    if (command == MONITOR_COMMAND_CHECK_ONCE) {
        RunAllChecks(&config);
        return 0;
    }

    if (command == MONITOR_COMMAND_RUN) {
        DWORD delay_ms = config.interval_seconds * 1000U;

        wprintf(L"Running checks every %lu seconds. Press Ctrl+C to stop.\n", config.interval_seconds);
        for (;;) {
            RunAllChecks(&config);
            Sleep(delay_ms);
        }
    }

    return 2;
}
