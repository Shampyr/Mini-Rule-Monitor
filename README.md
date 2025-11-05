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
.\build.ps1
.\build\mini-rule-monitor.exe validate-config --config config.yaml
.\build\mini-rule-monitor.exe list-checks --config config.yaml
.\build\mini-rule-monitor.exe check-once --config config.yaml
.\build\mini-rule-monitor.exe run --config config.yaml
```

`validate-config` loads the file and reports configuration errors without
running checks. `list-checks` prints configured check names and types.
`check-once` executes each check once. `run` repeats `check-once` with the
configured interval.

The same native tool can be launched through the Python wrapper:

```powershell
python monitor.py build
python monitor.py validate-config
python monitor.py list-checks
python monitor.py check-once
python monitor.py run
```

Use `--build` to rebuild before executing a command:

```powershell
python monitor.py --build check-once
```

## Configuration Format

The parser supports a deliberately small YAML subset:

- `app.interval_seconds`
- `app.log_file`
- `checks` items with `name`, `type`, `path`, `name` for processes, and
  `min_free_gb`

The supported check types are `file_exists`, `disk`, `process`, and `system`.

## Build

The build script expects MinGW GCC to be available as `gcc.exe` in `PATH`.
Check it before building:

```powershell
gcc --version
```

It compiles with C11, Unicode macros, `wmain` support, and strict warnings:

```text
-std=c11 -Wall -Wextra -Wpedantic -DUNICODE -D_UNICODE -municode
```

If PowerShell blocks local scripts, run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
```

## Troubleshooting

- `Could not open configuration file`: check the `--config` path.
- `Unknown check type`: use `file_exists`, `disk`, `process`, or `system`.
- `Process check requires process_name`: process checks use `process_name`.
- `could not append to log file`: check permissions for the log directory.

## Notes

This project intentionally implements a small YAML-style parser for the sample
configuration format. It is not a complete YAML implementation.
