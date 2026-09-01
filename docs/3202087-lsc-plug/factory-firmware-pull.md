# Factory firmware pull: LSC 3202087.2

How the stock flash image is read from this plug. Read-only; no writes.

The result is a single **opaque `.bin`** (full chip flash). It is for **restore only**, not for reading or editing like source. See [2-dump-firmware.md](2-dump-firmware.md#what-the-bin-is-and-is-not).

**Status (2026-08-28):** **full 2 MiB dump on disk** (`factory-20260828-2101-bk7238.bin`). Binaries live under `firmware/3202087-lsc-plug/` (tracked in git).

## Device and host

| Item | Value |
| --- | --- |
| Product | LSC Smart Connect, article **3202087.2** |
| Module | Tuya **T1-2S-NL** (blue daughterboard) |
| SoC (assumed for read) | Beken **BK7238** (`bk7238` family in ltchiptool) |
| Host | md@m2, macOS, UTC+2 |
| USB-TTL | FT232RL, **3.3 V** jumper |
| Serial port | `/dev/cu.usbserial-AG0KXO8J` |
| Power | **No 230 V**; **3.3 V** from USB-TTL adapter only |

UART pads and safety: [1-connect.md](1-connect.md). Wiring photos: [uart1.png](../images/uart1.png), [uart2.jpg](../images/uart2.jpg).

## Wiring used

Four wires soldered to the blue module (TX/RX crossed):

| Module pad | Adapter |
| --- | --- |
| 3V3 | VCC |
| GND | GND |
| RX1 | TXO |
| TX1 | RXI |

**CEN** was **not** connected in the first pull attempt. Guides require a short **CEN → GND** pulse (~0.25 s) to enter Beken download mode [Keet Support, sources.md](sources.md).

Prior session: adapter loopback (TXO ↔ RXI) **passed**; plug powered from adapter 3.3 V; no ASCII boot log at any baud (expected for this family).

## Tooling

This chip is **Beken**, not Espressif. **`esptool` does not apply.**

| Step | Command / path |
| --- | --- |
| Python venv | `python3 -m venv .venv` (repo root) |
| Install flasher | `.venv/bin/pip install -r requirements/ltchiptool.txt` |
| List families | `.venv/bin/ltchiptool list families` → `bk7238` supported |
| Read script | `firmware/3202087-lsc-plug/backup.sh` |
| Operator checklist | [2-dump-firmware.md](2-dump-firmware.md) |

## Pull command (read-only)

Default: full **2 MiB** flash at **115200** baud via ltchiptool CLI:

```bash
cd /path/to/reprogram-owned-devices
.venv/bin/ltchiptool flash read -d /dev/cu.usbserial-AG0KXO8J bk7238 \
  firmware/3202087-lsc-plug/factory-YYYYMMDD-HHMM-bk7238.bin
```

Or the helper (timestamped output, size check):

```bash
./firmware/3202087-lsc-plug/backup.sh
```

During the connect window: pulse **CEN → GND** once. If `bk7238` fails with CRC or timeout, retry with `FAMILY=bk7231n`.

## What happened on 2026-08-28

1. Resumed from UART-debug handoff ([1-connect.md](1-connect.md)): four-wire link OK; **3.3 V** adapter only (no **230 V**).
2. Created `firmware/3202087-lsc-plug/` workspace and installed **ltchiptool** in `.venv`.
3. Ran `ltchiptool flash read` with family **`bk7238`** and port `/dev/cu.usbserial-AG0KXO8J` **without a CEN wire**.
4. Read progressed to **1835008 bytes** (~87.5% of 2 MiB) then stalled; process killed.
5. Partial file **deleted** (not a valid backup).
6. **2026-08-28 21:01:** Full read with CEN tap succeeded, `factory-20260828-2101-bk7238.bin`, **2097152** bytes.

**Conclusion:** factory image backed up. Keep this file safe before any flash write.

## Verify a good dump

```bash
wc -c firmware/3202087-lsc-plug/factory-*.bin
shasum -a 256 firmware/3202087-lsc-plug/factory-*.bin
.venv/bin/ltchiptool flash file firmware/3202087-lsc-plug/factory-*.bin
```

| Check | Pass |
| --- | --- |
| Size | **2097152** bytes exactly |
| Partial / truncated | Re-run; do not keep |
| SHA-256 | Record in this file or a dated note when pull completes |

When a full dump exists, add a row here:

| Date | File | Family | Size | SHA-256 |
| --- | --- | --- | --- | --- |
| 2026-08-28 | `firmware/3202087-lsc-plug/factory-20260828-2101-bk7238.bin` | bk7238 | 2097152 | `3c0689f8ef0a15bc30ea85ab332cbe37667d3731f6084c8d333a02f6faa3ddfa` |

## Restore (reference only)

After a verified backup only:

```bash
.venv/bin/ltchiptool flash write -d /dev/cu.usbserial-AG0KXO8J -f bk7238 -s 0x0 -l 0x200000 \
  firmware/3202087-lsc-plug/factory-YYYYMMDD-HHMM-bk7238.bin
```

Raw 2 MiB dumps need `-f bk7238 -s 0x0 -l 0x200000` (see [5-flash.md](5-flash.md)). Same CEN pulse on connect. This **writes** flash; do not run until intentional.

## Related paths

- [0-feasibility.md](0-feasibility.md), hardware and flash settings
- [1-connect.md](1-connect.md), UART wiring and CEN
- [sources.md](sources.md), external guides (Keet, OpenBK, ESPHome Devices)
- [firmware/3202087-lsc-plug/](../../firmware/3202087-lsc-plug/), binaries and `backup.sh`
- [firmware/3202087-lsc-plug/uart-debug/](../../firmware/3202087-lsc-plug/uart-debug/), optional UART link verification (pre-flash prep)
