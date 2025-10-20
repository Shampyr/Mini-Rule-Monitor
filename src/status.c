#include "monitor.h"

const wchar_t *MonitorStatusText(MonitorStatus status)
{
    switch (status) {
    case MONITOR_STATUS_OK:
        return L"OK";
    case MONITOR_STATUS_WARN:
        return L"WARN";
    case MONITOR_STATUS_FAIL:
        return L"FAIL";
    case MONITOR_STATUS_SKIP:
        return L"SKIP";
    default:
        return L"UNKNOWN";
    }
}
