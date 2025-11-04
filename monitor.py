from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent
DEFAULT_EXE = ROOT / "build" / "mini-rule-monitor.exe"
DEFAULT_CONFIG = ROOT / "config.yaml"
BUILD_SCRIPT = ROOT / "build.ps1"


def build_native() -> int:
    if not BUILD_SCRIPT.exists():
        print(f"Build script not found: {BUILD_SCRIPT}", file=sys.stderr)
        return 2

    command = [
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        str(BUILD_SCRIPT),
    ]
    return subprocess.call(command, cwd=ROOT)


def run_native(args: argparse.Namespace) -> int:
    exe_path = Path(args.exe).resolve()
    config_path = Path(args.config).resolve()

    if args.build or not exe_path.exists():
        build_code = build_native()
        if build_code != 0:
            return build_code

    if not exe_path.exists():
        print(f"Native executable not found: {exe_path}", file=sys.stderr)
        print("Run: python monitor.py build", file=sys.stderr)
        return 2

    command = [str(exe_path), args.command, "--config", str(config_path)]
    return subprocess.call(command, cwd=ROOT)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="monitor.py",
        description="Python wrapper for the Mini Rule Monitor native CLI.",
    )
    parser.add_argument(
        "--config",
        default=str(DEFAULT_CONFIG),
        help="Path to config.yaml.",
    )
    parser.add_argument(
        "--exe",
        default=str(DEFAULT_EXE),
        help="Path to mini-rule-monitor.exe.",
    )
    parser.add_argument(
        "--build",
        action="store_true",
        help="Build native executable before running the command.",
    )
    parser.add_argument(
        "command",
        choices=["build", "validate-config", "list-checks", "check-once", "run"],
        help="Wrapper command to execute.",
    )

    args = parser.parse_args(argv)
    return args


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)

    if args.command == "build":
        return build_native()

    return run_native(args)


if __name__ == "__main__":
    raise SystemExit(main())
