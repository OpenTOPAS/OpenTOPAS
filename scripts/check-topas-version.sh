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
    match = re.search(rf"set \\(\\s*{name}\\s+([0-9]+)\\s*\\)", text)
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
errors = []

def expect_all_equal(values, label, path):
    if not values:
        errors.append(f"{path}: missing {label}")
        return
    unique = sorted(set(values))
    if unique != [tag]:
        errors.append(f"{path}: {label} {unique} != {tag}")

for path in paths:
    text = path.read_text(encoding="utf-8")
    if path.name.startswith("OpenTOPAS_quickStart_"):
        version_line = re.findall(
            r"TOPAS version \\*\\*(v\\d+\\.\\d+\\.\\d+)\\*\\*",
            text,
        )
        checkout_tags = re.findall(r"\\bgit checkout (v\\d+\\.\\d+\\.\\d+)\\b", text)
        app_tags = re.findall(r"\\bapps/topas-(v\\d+\\.\\d+\\.\\d+)\\.json\\b", text)
        expect_all_equal(version_line, "TOPAS version", path)
        expect_all_equal(checkout_tags, "git checkout tag", path)
        expect_all_equal(app_tags, "apps/topas tag", path)
    elif path.name == "Dockerfile.topas.workflow":
        desc_tags = re.findall(r"TOPAS (v\\d+\\.\\d+\\.\\d+)", text)
        arg_tags = re.findall(r"ARG TOPAS_VERSION=(v\\d+\\.\\d+\\.\\d+)", text)
        expect_all_equal(desc_tags, "description tag", path)
        expect_all_equal(arg_tags, "ARG TOPAS_VERSION", path)
    elif path.name in {"README.Docker.md", "FAQ.Docker.md"}:
        docker_tags = re.findall(r"OpenTOPAS (v\\d+\\.\\d+\\.\\d+)", text)
        expect_all_equal(docker_tags, "OpenTOPAS tag", path)

if errors:
    print("TOPAS version mismatch detected:")
    for err in errors:
        print(f"- {err}")
    raise SystemExit(1)

print(f"TOPAS version references are consistent: {tag}")
PY
