#!/usr/bin/env bash
# Read-only factory flash backup for LSC 3202087.2 (Beken).
# Usage: ./firmware/3202087-lsc-plug/backup.sh
# Env:   PORT=/dev/cu.usbserial-XXX  FAMILY=bk7238|bk7231n

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
VENV_LT="$ROOT/.venv/bin/ltchiptool"
OUT_DIR="$ROOT/firmware/3202087-lsc-plug"
PORT="${PORT:-}"
FAMILY="${FAMILY:-bk7238}"
EXPECTED_SIZE=2097152

if [[ ! -x "$VENV_LT" ]]; then
  echo "ltchiptool not found. Run:" >&2
  echo "  python3 -m venv $ROOT/.venv && $ROOT/.venv/bin/pip install -r $ROOT/requirements/ltchiptool.txt" >&2
  exit 1
fi

if [[ -z "$PORT" ]]; then
  PORT="$(ls /dev/cu.usbserial* 2>/dev/null | head -1 || true)"
fi
if [[ -z "$PORT" ]]; then
  echo "No USB serial port found. Set PORT=/dev/cu.usbserial-XXX" >&2
  exit 1
fi

if [[ $(ls /dev/cu.usbserial* 2>/dev/null | wc -l | tr -d ' ') -gt 1 ]]; then
  echo "Multiple serial ports; set PORT explicitly." >&2
  ls /dev/cu.usbserial* >&2
  exit 1
fi

STAMP="$(date +%Y%m%d-%H%M)"
OUT="$OUT_DIR/factory-${STAMP}-${FAMILY}.bin"

mkdir -p "$OUT_DIR"

echo "Read-only backup -> $OUT"
echo "Port: $PORT  Family: $FAMILY  Expected size: $EXPECTED_SIZE bytes"
echo ""
echo "=== CEN: get ready ==="
echo "  1. Loose wire on CEN pad (blue module Side B, back edge, not green PCB)"
echo "  2. Start command below; when ltchiptool prints connection text,"
echo "     tap CEN -> GND for ~0.25 s (use adapter GND or header GND)"
echo "  3. No 230 V on plug, 3.3 V from adapter only"
echo ""
read -r -p "CEN wire ready? Press Enter to start (Ctrl+C to abort)... " _
echo ""
echo ">>> TAP CEN -> GND when ltchiptool tries to connect <<<"
echo ""

"$VENV_LT" flash read -d "$PORT" "$FAMILY" "$OUT"

SIZE="$(wc -c < "$OUT" | tr -d ' ')"
if [[ "$SIZE" != "$EXPECTED_SIZE" ]]; then
  echo "ERROR: dump size $SIZE != $EXPECTED_SIZE. Delete partial file and retry with CEN pulse." >&2
  exit 1
fi

echo ""
echo "OK: $OUT ($SIZE bytes)"
shasum -a 256 "$OUT"
