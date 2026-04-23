from __future__ import annotations

import datetime as _datetime
import os
import subprocess

Import("env")  # type: ignore[name-defined]


def _run_git(project_dir: str, *args: str) -> str:
    try:
        return (
            subprocess.check_output(["git", *args], cwd=project_dir)
            .decode("utf-8", errors="replace")
            .strip()
        )
    except Exception:
        return ""


def _quote_macro(value: str) -> str:
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'\\"{escaped}\\"'


project_dir = env.subst("$PROJECT_DIR")
version = os.environ.get("FIRMWARE_VERSION") or _run_git(
    project_dir, "describe", "--tags", "--dirty", "--always"
)
if not version:
    version = "dev"

git_sha = os.environ.get("FIRMWARE_GIT_SHA") or _run_git(
    project_dir, "rev-parse", "--short=12", "HEAD"
)
if not git_sha:
    git_sha = "unknown"

build_time = os.environ.get("FIRMWARE_BUILD_TIME")
if not build_time:
    build_time = (
        _datetime.datetime.now(_datetime.timezone.utc)
        .replace(microsecond=0)
        .isoformat()
        .replace("+00:00", "Z")
    )

env.Append(
    CPPDEFINES=[
        ("FIRMWARE_VERSION", _quote_macro(version)),
        ("FIRMWARE_GIT_SHA", _quote_macro(git_sha)),
        ("FIRMWARE_BUILD_TIME", _quote_macro(build_time)),
    ]
)
