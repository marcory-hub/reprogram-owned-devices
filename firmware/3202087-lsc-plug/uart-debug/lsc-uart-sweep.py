#!/usr/bin/env python3
"""Sweep UART baud rates on one USB power-on. Plug USB when prompted."""
import os
import re
import struct
import sys
import termios
import time
import fcntl

PORT = sys.argv[1] if len(sys.argv) > 1 else "/dev/cu.usbserial-AG0KXO8J"
BAUDS = [74880, 115200, 57600, 38400, 9600, 19200, 460800, 921600]
SECONDS_PER_BAUD = 3

IOSSIOSPEED = 0x80045402
SPEED_MAP = {
    9600: termios.B9600,
    19200: termios.B19200,
    38400: termios.B38400,
    57600: termios.B57600,
    115200: termios.B115200,
    460800: getattr(termios, "B460800", None),
    921600: getattr(termios, "B921600", None),
}


def set_baud(fd: int, baud: int) -> None:
    attrs = termios.tcgetattr(fd)
    attrs[0] = attrs[1] = attrs[3] = 0
    attrs[2] = termios.CS8 | getattr(termios, "CLOCAL", 0)
    attrs[6][termios.VMIN] = 0
    attrs[6][termios.VTIME] = 0
    speed = SPEED_MAP.get(baud)
    if baud == 74880:
        attrs[4] = attrs[5] = termios.B115200
        termios.tcsetattr(fd, termios.TCSANOW, attrs)
        fcntl.ioctl(fd, IOSSIOSPEED, struct.pack("I", 74880))
    elif speed is None:
        raise ValueError(f"unsupported baud {baud}")
    else:
        attrs[4] = attrs[5] = speed
        termios.tcsetattr(fd, termios.TCSANOW, attrs)
    termios.tcflush(fd, termios.TCIOFLUSH)


def capture(port: str, baud: int, seconds: float) -> bytes:
    fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    try:
        set_baud(fd, baud)
        end = time.time() + seconds
        chunks = []
        while time.time() < end:
            try:
                data = os.read(fd, 4096)
                if data:
                    chunks.append(data)
            except BlockingIOError:
                time.sleep(0.02)
        return b"".join(chunks)
    finally:
        os.close(fd)


def score(blob: bytes) -> tuple[int, int, str]:
  if not blob:
      return 0, 0, ""
  printable = sum(1 for b in blob if b in range(0x20, 0x7F) or b in (0x09, 0x0A, 0x0D))
  text = re.sub(rb"[^\x20-\x7e\r\n\t]", b".", blob).decode("ascii", errors="replace")
  return len(blob), printable, text[:120]


def main() -> None:
    print(f"Unplug USB now. Plug in within 30 s, waiting for {PORT}...")
    for _ in range(60):
        if os.path.exists(PORT):
            break
        time.sleep(0.5)
    if not os.path.exists(PORT):
        raise SystemExit(f"Port not found: {PORT}")
    time.sleep(0.3)
    print("Port up. Sweeping baud rates on this boot window...\n")

    best = (0, 0, 0, "", b"")
    for baud in BAUDS:
        try:
            blob = capture(PORT, baud, SECONDS_PER_BAUD)
        except ValueError as exc:
            print(f"{baud:>7}  skip ({exc})")
            continue
        nbytes, printable, preview = score(blob)
        print(f"{baud:>7}  bytes={nbytes:>4}  printable={printable:>3}  {preview!r}")
        if printable > best[1] or (printable == best[1] and nbytes > best[0]):
            best = (nbytes, printable, baud, preview, blob)

    print()
    if best[1] >= 10:
        print(f"BEST: {best[2]} baud ({best[1]} printable chars)")
        print(best[3])
    else:
        print("No readable ASCII at any tested baud.")
        print("Next: swap white/yellow at ADAPTER header only, rerun this script.")
        if best[4]:
            print(f"Raw hex ({best[2]} baud, first 64 bytes):")
            print(best[4][:64].hex(" "))


if __name__ == "__main__":
    main()
