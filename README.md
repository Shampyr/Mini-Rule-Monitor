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

## Commands

```powershell
.\build\mini-rule-monitor.exe validate-config --config config.yaml
.\build\mini-rule-monitor.exe list-checks --config config.yaml
.\build\mini-rule-monitor.exe check-once --config config.yaml
.\build\mini-rule-monitor.exe run --config config.yaml
```

`validate-config` loads the file and reports configuration errors without
running checks. `list-checks` prints configured check names and types.
`check-once` executes each check once. `run` repeats `check-once` with the
configured interval.

## Configuration Format

The parser supports a deliberately small YAML subset:

- `app.interval_seconds`
- `app.log_file`
- `checks` items with `name`, `type`, `path`, `name` for processes, and
  `min_free_gb`

The supported check types are `file_exists`, `disk`, `process`, and `system`.
