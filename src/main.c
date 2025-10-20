#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include "monitor.h"

int wmain(int argc, wchar_t **argv)
{
    (void)argc;
    (void)argv;

    wprintf(L"Mini Rule Monitor\n");
    wprintf(L"Use validate-config, list-checks, check-once, or run.\n");
    return 0;
}
