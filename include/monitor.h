#ifndef MINI_RULE_MONITOR_H
#define MINI_RULE_MONITOR_H

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <strsafe.h>
#include <wchar.h>

#define MRM_MAX_CHECKS 64
#define MRM_MAX_NAME 128
#define MRM_MAX_TYPE 32
#define MRM_MAX_MESSAGE 512
#define MRM_DEFAULT_INTERVAL_SECONDS 60U
#define MRM_DEFAULT_CONFIG_PATH L"config.yaml"

typedef enum MonitorStatusTag {
    MONITOR_STATUS_OK = 0,
    MONITOR_STATUS_WARN = 1,
    MONITOR_STATUS_FAIL = 2,
    MONITOR_STATUS_SKIP = 3
} MonitorStatus;

typedef struct MonitorCheckTag {
    wchar_t name[MRM_MAX_NAME];
    wchar_t type[MRM_MAX_TYPE];
    wchar_t path[MAX_PATH];
    wchar_t process_name[MAX_PATH];
    double min_free_gb;
    DWORD line_number;
} MonitorCheck;

typedef struct MonitorConfigTag {
    DWORD interval_seconds;
    wchar_t log_file[MAX_PATH];
    MonitorCheck checks[MRM_MAX_CHECKS];
    DWORD check_count;
} MonitorConfig;

typedef struct ConfigErrorTag {
    DWORD line_number;
    wchar_t message[MRM_MAX_MESSAGE];
} ConfigError;

typedef struct CheckResultTag {
    wchar_t name[MRM_MAX_NAME];
    wchar_t type[MRM_MAX_TYPE];
    MonitorStatus status;
    wchar_t message[MRM_MAX_MESSAGE];
} CheckResult;

typedef enum MonitorCommandTag {
    MONITOR_COMMAND_HELP = 0,
    MONITOR_COMMAND_VALIDATE_CONFIG,
    MONITOR_COMMAND_LIST_CHECKS,
    MONITOR_COMMAND_CHECK_ONCE,
    MONITOR_COMMAND_RUN
} MonitorCommand;

const wchar_t *MonitorStatusText(MonitorStatus status);
void MonitorConfigInit(MonitorConfig *config);
BOOL LoadMonitorConfig(LPCWSTR path, MonitorConfig *config, ConfigError *error);
BOOL RunMonitorCheck(const MonitorCheck *check, CheckResult *result);
void PrintCheckResult(const CheckResult *result);
BOOL AppendCheckResultToLog(LPCWSTR log_file, const CheckResult *result);

#endif
