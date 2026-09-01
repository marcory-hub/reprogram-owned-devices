#!/usr/bin/env bash
# LSC Smart Connect UART link verification (host-side only).
# Operator must complete Phase 1 multimeter checks before powering the target.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PORT="${1:-/dev/cu.usbserial-AG0KXO8J}"
OUT_DIR="${2:-$SCRIPT_DIR/uart-capture}"
mkdir -p "$OUT_DIR"

echo "=== Phase 2.1: serial port enumeration ==="
ls -la /dev/cu.usbserial* 2>/dev/null || { echo "No cu.usbserial devices found"; exit 1; }

if [[ ! -e "$PORT" ]]; then
  echo "Port not found: $PORT"
  exit 1
fi

echo
echo "=== Phase 3: boot capture (power-cycle target during each capture) ==="
echo "Port: $PORT"
echo "Output: $OUT_DIR"
echo

capture_baud() {
  local baud="$1"
  local out="$OUT_DIR/boot_${baud}.bin"
  local txt="$OUT_DIR/boot_${baud}.txt"
  echo "--- baud $baud ---"
  if ! stty -f "$PORT" "$baud" cs8 -cstopb -parenb raw -echo 2>/dev/null; then
    echo "stty rejected $baud; using Python termios"
    python3 - "$PORT" "$baud" "$out" <<'PY'
import sys, termios, os, time, struct, fcntl
port, baud_s, out_path = sys.argv[1], sys.argv[2], sys.argv[3]
baud = int(baud_s)
speed_map = {
    115200: termios.B115200,
    57600: termios.B57600,
    38400: termios.B38400,
    9600: termios.B9600,
}
fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
try:
    attrs = termios.tcgetattr(fd)
    attrs[0] = 0
    attrs[1] = 0
    speed = speed_map.get(baud)
    if speed is None and baud == 74880:
        attrs[2] = termios.CS8 | getattr(termios, "CLOCAL", 0)
        attrs[3] = 0
        attrs[4] = attrs[5] = termios.B115200
        attrs[6][termios.VMIN] = 0
        attrs[6][termios.VTIME] = 0
        termios.tcsetattr(fd, termios.TCSANOW, attrs)
        IOSSIOSPEED = 0x80045402
        fcntl.ioctl(fd, IOSSIOSPEED, struct.pack("I", 74880))
    elif speed is None:
        print(f"unsupported baud {baud}", file=sys.stderr)
        sys.exit(2)
    else:
        attrs[2] = speed | termios.CS8 | getattr(termios, "CLOCAL", 0)
        attrs[3] = 0
        attrs[4] = attrs[5] = speed
        attrs[6][termios.VMIN] = 0
        attrs[6][termios.VTIME] = 0
        termios.tcsetattr(fd, termios.TCSANOW, attrs)
    termios.tcflush(fd, termios.TCIOFLUSH)
    end = time.time() + 8
    chunks = []
    while time.time() < end:
        try:
            data = os.read(fd, 4096)
            if data:
                chunks.append(data)
        except BlockingIOError:
            time.sleep(0.05)
    blob = b"".join(chunks)
finally:
    os.close(fd)
open(out_path, "wb").write(blob)
print(f"bytes={len(blob)}")
PY
  else
    perl -e '
      use Fcntl qw(F_GETFL F_SETFL O_NONBLOCK);
      my ($port, $baud, $out) = @ARGV;
      sysopen my $fh, $port, O_RDWR | O_NOCTTY | O_NONBLOCK or die $!;
      my $end = time() + 8;
      my $blob = "";
      while (time() < $end) {
        my $n = sysread $fh, my $buf, 4096;
        $blob .= $buf if defined $n && $n > 0;
        select undef, undef, undef, 0.05;
      }
      open my $of, ">:raw", $out or die $!;
      print $of $blob;
      print "bytes=", length($blob), "\n";
    ' "$PORT" "$baud" "$out"
  fi
  python3 - "$out" "$txt" <<'PY'
import sys, re
raw = open(sys.argv[1], "rb").read()
open(sys.argv[2], "w", encoding="utf-8", errors="replace").write(
    re.sub(rb"[^\x20-\x7e\r\n\t]", b"?", raw).decode("ascii", errors="replace")
)
print("preview:", repr(raw[:120]))
PY
}

for b in 115200 74880 9600 38400 57600; do
  capture_baud "$b"
  echo
done

echo "=== summary ==="
for f in "$OUT_DIR"/boot_*.txt; do
  [[ -f "$f" ]] || continue
  bytes=$(wc -c < "${f%.txt}.bin" | tr -d ' ')
  printable=$(tr -cd '[:print:]\n\t' < "$f" | wc -c | tr -d ' ')
  echo "$(basename "$f"): raw_bytes=$bytes printable_chars=$printable"
  head -c 200 "$f" | tr '\n' ' '
  echo
done
