#!/usr/bin/env bash
# Restore Elecrow SAD00006D factory firmware from published vendor bins.
# Prerequisite: docs/SAD00006D-ai-cam/mac-host-setup.md (CH340 driver + repo .venv).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
FACTORY="${ROOT}/firmware/SAD00006D-ai-cam/factory-firmware/AICamera_ESP32S3_PCBA-V1.1_260629"
ESPTOOL="${ROOT}/.venv/bin/esptool.py"

if [[ ! -x "$ESPTOOL" ]]; then
  echo "esptool missing. From repo root: python3 -m venv .venv && .venv/bin/pip install -r requirements/esptool.txt" >&2
  exit 1
fi

PORT="${1:-}"
if [[ -z "$PORT" ]]; then
  PORT="$(ls /dev/cu.usbmodem* /dev/cu.usbserial* /dev/cu.wchusbserial* 2>/dev/null | head -1 || true)"
fi
if [[ -z "$PORT" ]]; then
  echo "No serial port found. Plug in the camera, install the CH340 driver if needed, then pass PORT explicitly." >&2
  echo "  ls /dev/cu.usbmodem* /dev/cu.usbserial* /dev/cu.wchusbserial* 2>/dev/null" >&2
  exit 1
fi

for f in \
  "${FACTORY}/AI_Camera-V1.0-Arduino.ino.bootloader.bin" \
  "${FACTORY}/AI_Camera-V1.0-Arduino.ino.partitions.bin" \
  "${FACTORY}/boot_app0.bin" \
  "${FACTORY}/AI_Camera-V1.0-Arduino.ino.bin"
do
  if [[ ! -f "$f" ]]; then
    echo "Missing factory bin: $f" >&2
    exit 1
  fi
done

echo "Port: $PORT"
echo "Flashing factory firmware (ESP32-S3)..."

"$ESPTOOL" --chip esp32s3 -p "$PORT" -b 460800 \
  write_flash --flash_mode dio --flash_size 16MB \
  0x0 "${FACTORY}/AI_Camera-V1.0-Arduino.ino.bootloader.bin" \
  0x8000 "${FACTORY}/AI_Camera-V1.0-Arduino.ino.partitions.bin" \
  0xe000 "${FACTORY}/boot_app0.bin" \
  0x10000 "${FACTORY}/AI_Camera-V1.0-Arduino.ino.bin"

echo "Done. Press RESET on the board if the display does not update."
