#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmake_file="$root/CMakeLists.txt"

version="$(
python3 - "$cmake_file" <<'PY'
import re
import sys

text = open(sys.argv[1], "r", encoding="utf-8").read()

def get(name: str) -> str:
    match = re.search(rf"set\\s*\\(\\s*{name}\\s+([0-9]+)\\s*\\)", text)
    if not match:
        raise SystemExit(f"Missing {name} in {sys.argv[1]}")
    return match.group(1)

major = get("TOPAS_VERSION_MAJOR")
minor = get("TOPAS_VERSION_MINOR")
patch = get("TOPAS_VERSION_PATCH")
print(f"{major}.{minor}.{patch}")
PY
)"

tag="v${version}"

python3 - "$tag" \
  "$root/OpenTOPAS_quickStart_Debian.md" \
  "$root/OpenTOPAS_quickStart_MacOS.md" \
  "$root/OpenTOPAS_quickStart_WSL.md" \
  "$root/docker/README.Docker.md" \
  "$root/docker/FAQ.Docker.md" \
  "$root/.github/workflows/Dockerfile.topas.workflow" <<'PY'
import re
import sys
from pathlib import Path

tag = sys.argv[1]
paths = [Path(p) for p in sys.argv[2:]]

for path in paths:
    text = path.read_text(encoding="utf-8")
    updated = text
    if path.name.startswith("OpenTOPAS_quickStart_"):
        updated = re.sub(
            r"(TOPAS version \\*\\*)v\\d+\\.\\d+\\.\\d+(\\*\\*)",
            rf"\\1{tag}\\2",
            updated,
        )
        updated = re.sub(
            r"\\bgit checkout v\\d+\\.\\d+\\.\\d+\\b",
            f"git checkout {tag}",
            updated,
        )
        updated = re.sub(
            r"\\bapps/topas-v\\d+\\.\\d+\\.\\d+\\.json\\b",
            f"apps/topas-{tag}.json",
            updated,
        )
    elif path.name == "Dockerfile.topas.workflow":
        updated = re.sub(
            r"TOPAS v\\d+\\.\\d+\\.\\d+",
            f"TOPAS {tag}",
            updated,
        )
        updated = re.sub(
            r"ARG TOPAS_VERSION=v\\d+\\.\\d+\\.\\d+",
            f"ARG TOPAS_VERSION={tag}",
            updated,
        )
    elif path.name in {"README.Docker.md", "FAQ.Docker.md"}:
        updated = re.sub(
            r"OpenTOPAS v\\d+\\.\\d+\\.\\d+",
            f"OpenTOPAS {tag}",
            updated,
        )

    if updated != text:
        path.write_text(updated, encoding="utf-8")
PY

echo "Updated TOPAS version references to ${tag}."
