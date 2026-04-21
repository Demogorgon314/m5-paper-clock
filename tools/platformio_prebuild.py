Import("env")

from pathlib import Path
import os
import shlex
import shutil
import subprocess


PROJECT_DIR = Path(env["PROJECT_DIR"])
UPDATE_SCRIPT = PROJECT_DIR / "tools" / "update_holiday_data.mjs"
GENERATED_FILES = [PROJECT_DIR / "include" / "logic" / "HolidayData.h"]


def generated_files_ready():
    return all(path.exists() for path in GENERATED_FILES)


def resolve_year_args():
    years_value = os.environ.get("HOLIDAY_ASSET_YEARS", "").strip()
    if not years_value:
        return []
    return shlex.split(years_value)


def main():
    node_path = shutil.which("node")
    if not node_path:
        if generated_files_ready():
            print("Warning: `node` not found, using existing holiday data.")
            return
        raise RuntimeError("`node` is required to generate holiday data.")

    command = [node_path, str(UPDATE_SCRIPT)] + resolve_year_args()
    try:
        print("Auto-updating holiday data before PlatformIO build...")
        subprocess.run(command, cwd=str(PROJECT_DIR), check=True)
    except subprocess.CalledProcessError as exc:
        if generated_files_ready():
            print("Warning: holiday data refresh failed; using existing generated file.")
            print(f"Warning detail: {exc}")
            return
        raise RuntimeError("holiday data refresh failed and no cached file exists")


main()
