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

int wmain(int argc, wchar_t **argv)
{
    MonitorCommand command = MONITOR_COMMAND_HELP;
    LPCWSTR config_path = MRM_DEFAULT_CONFIG_PATH;

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
    wprintf(L"Command: %d\n", (int)command);
    wprintf(L"Config: %ls\n", config_path);
    return 0;
}
