#!/usr/bin/env bash
# Non-interactive boot capture: starts listening, you power-cycle USB within 15 s.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PORT="${1:-/dev/cu.usbserial-AG0KXO8J}"
BAUD="${2:-115200}"
OUT="${3:-$SCRIPT_DIR/uart-capture/powercycle_${BAUD}.bin}"

mkdir -p "$(dirname "$OUT")"

echo "Listening on $PORT at $BAUD for 15 s."
echo "Unplug USB now, wait 2 s, replug within the next 15 s window."
echo "Output: $OUT"
echo

python3 - "$PORT" "$BAUD" "$OUT" <<'PY'
import sys, termios, os, time, struct, fcntl
port, baud_s, out_path = sys.argv[1], sys.argv[2], sys.argv[3]
baud = int(baud_s)
speed_map = {115200: termios.B115200, 57600: termios.B57600, 38400: termios.B38400, 9600: termios.B9600}
fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
try:
    attrs = termios.tcgetattr(fd)
    attrs[0] = attrs[1] = attrs[3] = 0
    speed = speed_map.get(baud)
    if speed is None and baud == 74880:
        attrs[2] = termios.CS8 | getattr(termios, "CLOCAL", 0)
        attrs[4] = attrs[5] = termios.B115200
        attrs[6][termios.VMIN] = 0
        attrs[6][termios.VTIME] = 0
        termios.tcsetattr(fd, termios.TCSANOW, attrs)
        fcntl.ioctl(fd, 0x80045402, struct.pack("I", 74880))
    elif speed is None:
        raise SystemExit(f"unsupported baud {baud}")
    else:
        attrs[2] = speed | termios.CS8 | getattr(termios, "CLOCAL", 0)
        attrs[4] = attrs[5] = speed
        attrs[6][termios.VMIN] = 0
        attrs[6][termios.VTIME] = 0
        termios.tcsetattr(fd, termios.TCSANOW, attrs)
    termios.tcflush(fd, termios.TCIOFLUSH)
    end = time.time() + 15
    chunks = []
    while time.time() < end:
        try:
            data = os.read(fd, 4096)
            if data:
                chunks.append(data)
        except BlockingIOError:
            time.sleep(0.02)
    blob = b"".join(chunks)
finally:
    os.close(fd)
open(out_path, "wb").write(blob)
print(f"bytes={len(blob)}")
if blob:
    import re
    text = re.sub(rb"[^\x20-\x7e\r\n\t]", b"?", blob).decode("ascii", errors="replace")
    print(text[:500])
PY

echo
echo "Hex preview:"
xxd "$OUT" | head -20
