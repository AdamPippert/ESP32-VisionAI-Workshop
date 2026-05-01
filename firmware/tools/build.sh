#!/usr/bin/env bash
# build.sh — Build (and optionally flash) an ESP32-S3 lab project.
#
# Usage:
#   bash firmware/tools/build.sh lab_01               # build only
#   bash firmware/tools/build.sh lab_02 flash         # build + flash
#   bash firmware/tools/build.sh camera_test flash    # build + flash camera_test
#
# The script auto-detects the ESP-IDF Python venv and toolchain.

set -euo pipefail

LAB="${1:-}"
ACTION="${2:-build}"   # build | flash | build flash

if [ -z "$LAB" ]; then
  echo "Usage: $0 <lab_name> [build|flash]"
  echo "  e.g. $0 lab_02 flash"
  exit 1
fi

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PROJECT_DIR="$REPO_ROOT/firmware/$LAB"

if [ ! -d "$PROJECT_DIR" ]; then
  echo "ERROR: $PROJECT_DIR does not exist"
  exit 1
fi

IDF_PATH="$HOME/esp/esp-idf"
VENV="$HOME/.espressif/python_env/idf5.4_py3.13_env"
TOOLCHAIN="$HOME/.espressif/tools/xtensa-esp-elf/esp-14.2.0_20241119/xtensa-esp-elf/bin"
ROM_ELF_DIR="$HOME/.espressif/tools/esp-rom-elfs/20241011"
PORT="${PORT:-/dev/cu.usbserial-110}"

export IDF_PATH IDF_PYTHON_ENV_PATH="$VENV" ESP_ROM_ELF_DIR="$ROM_ELF_DIR"
export PATH="$TOOLCHAIN:$PATH"

cd "$PROJECT_DIR"
"$VENV/bin/python" "$IDF_PATH/tools/idf.py" -p "$PORT" $ACTION
