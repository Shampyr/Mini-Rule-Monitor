# Mini Rule Monitor

Mini Rule Monitor is a small Windows command line monitor written in C11.
It reads a simple YAML-style configuration file, runs local checks, prints
results to the console, and can append the same results to a log file.

## Planned MVP

- Validate and list configured checks.
- Run checks once or continuously.
- Check files, disk space, running processes, and basic system memory.
- Use Unicode-aware WinAPI calls throughout the native code.

HTTP monitoring is planned for a later version.
