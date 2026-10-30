#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
project_file="$project_dir/WizardOs.uproject"
editor="$HOME/UnrealEngine/Engine/Binaries/Linux/UnrealEditor"

if [[ $# -lt 2 ]]; then
    echo "Usage: bash Scripts/import_asset.sh /Game/Folder /absolute/path/to/file [more files...]" >&2
    exit 1
fi

destination="$1"
shift

if [[ "$destination" != /Game/* ]]; then
    echo "Destination must begin with /Game/, for example /Game/UI/Imports." >&2
    exit 1
fi

if ps -eo args= | grep -F "$project_file" | grep -q '[U]nrealEditor'; then
    echo "Close the WizardOs Unreal Editor before importing assets." >&2
    exit 1
fi

files=()
for file in "$@"; do
    if [[ ! -f "$file" ]]; then
        echo "File not found: $file" >&2
        exit 1
    fi
    files+=("$(realpath "$file")")
done

script="$(mktemp --suffix=.py)"
trap 'rm -f "$script"' EXIT

python3 - "$script" "$destination" "${files[@]}" <<'PYTHON'
import json
import pathlib
import sys

script_path = pathlib.Path(sys.argv[1])
destination = sys.argv[2]
files = sys.argv[3:]

script_path.write_text(
    """import unreal

files = {files}
tasks = []
for filename in files:
    task = unreal.AssetImportTask()
    task.filename = filename
    task.destination_path = {destination}
    task.automated = True
    task.replace_existing = True
    task.save = True
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
unreal.EditorAssetLibrary.save_directory({destination}, only_if_is_dirty=False, recursive=True)
unreal.SystemLibrary.quit_editor()
""".format(
        files=json.dumps(files),
        destination=json.dumps(destination),
    )
)
PYTHON

"$editor" "$project_file" "-ExecutePythonScript=$script" -NoSound -nosplash -nop4
