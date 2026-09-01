# LSC Smart Connect 3202087.2: flash

**Device folder:** `3202087-lsc-plug`

Prerequisite: write-firmware notes in [4-write-firmware.md](4-write-firmware.md). Factory backup in `firmware/3202087-lsc-plug/` must exist and stay untouched.

## Prerequisites (once per machine)

From repo root:

**ltchiptool** (flash and restore):

```bash
python3 -m venv .venv
.venv/bin/pip install -r requirements/ltchiptool.txt
```

If you do not have a factory dump yet, pull one first ([2-dump-firmware.md](2-dump-firmware.md)). This demo uses the checked-in `factory-20260828-2101-bk7238.bin`.

**PlatformIO + LibreTiny** (build Morse SOS only; do not use `pio run -t upload`):

```bash
pip install -r requirements/platformio.txt
pio pkg install -d firmware/3202087-lsc-plug/morse-sos --platform libretiny
```

The first `pio run` in the Morse project may download the LibreTiny platform and toolchains; allow a few minutes.

## Current outcome

| Item | Status |
| --- | --- |
| Factory dump | **On disk**: `factory-20260828-2101-bk7238.bin` (2097152 bytes, SHA-256 unchanged) |
| Factory restore (2026-08-30) | **Succeeded**: full 2 MiB write via ltchiptool |
| Morse SOS firmware (2026-08-30) | **Flashed**: LibreTiny `firmware.uf2` on `t1-2s` |
| On-device behavior | Morse SOS (... --- ...) on **P9 / D-K relay LED** ([firmware/3202087-lsc-plug/morse-sos/](../../firmware/3202087-lsc-plug/morse-sos/)) |
| Rollback | Restore factory `.bin` below |

## 1. Serial port check

```bash
ls /dev/cu.usbserial*
```

Confirm the port is free. Stop if ambiguous or busy.

Example from 2026-08-30 session: `/dev/cu.usbserial-AG0KXO8J`.

## 2. Restore stock

Raw factory dump needs explicit family and offset:

```bash
.venv/bin/ltchiptool flash write -d PORT -f bk7238 -s 0x0 -l 0x200000 \
  firmware/3202087-lsc-plug/factory-20260828-2101-bk7238.bin
```

Replace `PORT` with your USB serial device (from step 1).

During connect: pulse **CEN** (blue module **Side B**, back edge) to **GND** ~0.25 s. See [1-connect.md](1-connect.md).

## 3. Custom firmware (Morse SOS)

Project: [firmware/3202087-lsc-plug/morse-sos/](../../firmware/3202087-lsc-plug/morse-sos/) (`board = t1-2s`, GPIO P9, `LED_ACTIVE_LOW`).

Build (no upload):

```bash
cd firmware/3202087-lsc-plug/morse-sos && pio run
```

Flash the UF2 only (auto-detect; do not pass `-f`/`-s`/`-l`):

```bash
.venv/bin/ltchiptool flash write -d PORT \
  firmware/3202087-lsc-plug/morse-sos/.pio/build/t1-2s/firmware.uf2
```

Same **CEN** pulse on connect. Do not use `pio run -t upload` or `esptool`.

### Why `firmware.uf2` (not other build outputs)

LibreTiny emits several files under `.pio/build/t1-2s/`. For UART flash on this Tuya T1 module, use **`firmware.uf2` only**:

| File | Use on this plug |
| --- | --- |
| `firmware.uf2` | **Yes**: ltchiptool auto-detects BK7238, writes the app at 0x011000, keeps the Tuya bootloader |
| `firmware.bin` | Same bytes as the UF2; easy to flash at the wrong offset by mistake |
| `image_bk7238_app.0x011000.rbl` | No, ltchiptool does not auto-flash this; manual offset risks overwriting the bootloader |
| `image_bk7238_app.0x011000.crc` | No, CRC’d app slice; not a standalone UART image ([Beken output files](https://docs.libretiny.eu/docs/platform/beken-72xx/)) |
| `image_bk7238_app.ota.ug.bin` | No, Cloudcutter OTA; does not apply to this generation |
| OpenBeken QIO/UA | No, can replace the Tuya bootloader and leave the device in a bad state |

Restore factory stock **before** the Morse write when coming from non-stock firmware. Full procedure: factory write (step 2), verify SOS cleared, then Morse UF2 (step 3).

## 4. Post-flash verification

**Stock restore success:** leftover non-stock SOS on the status LED stops after power-cycle on 3.3 V only. Full Tuya app/relay behavior needs mains; do not plug into 230 V for UART work.

**Morse SOS success:** after power-cycle on 3.3 V, Morse SOS visible on **P9 / D-K** (relay LED). **P11 / D-WIFI** is not the SOS source. Relay (P24) stays off. If P9 is dark or inverted, flip `LED_ACTIVE_LOW` in `src/main.cpp`, rebuild, and rewrite the UF2 only.

## Session log (2026-08-30)

Host: macOS. Power: USB-TTL **3.3 V** only (no 230 V). Port: `/dev/cu.usbserial-AG0KXO8J`. Tool: ltchiptool v4.14.4.

**Factory restore** (cleared leftover status-LED SOS from prior OpenBeken session):

```bash
.venv/bin/ltchiptool flash write -d /dev/cu.usbserial-AG0KXO8J -f bk7238 -s 0x0 -l 0x200000 \
  firmware/3202087-lsc-plug/factory-20260828-2101-bk7238.bin
```

Result: BK7238 linked, 2 MiB written, finished in ~149 s.

**Morse SOS** (LibreTiny Arduino, no Wi-Fi / relay):

```bash
.venv/bin/ltchiptool flash write -d /dev/cu.usbserial-AG0KXO8J \
  firmware/3202087-lsc-plug/morse-sos/.pio/build/t1-2s/firmware.uf2
```

Result: detected `UF2 - morse-sos 26.08.30`, board `t1-2s`, finished in ~27 s.

Factory backup after both writes: 2097152 bytes, SHA-256 `3c0689f8ef0a15bc30ea85ab332cbe37667d3731f6084c8d333a02f6faa3ddfa`.

## Safety

- Never erase or overwrite the factory backup file in place
- Stop if the command would wipe partitions not backed up
- **No 230 V** during UART flash

## References

- [1-connect.md](1-connect.md)
- [2-dump-firmware.md](2-dump-firmware.md)
- [4-write-firmware.md](4-write-firmware.md)
- [factory-firmware-pull.md](factory-firmware-pull.md)
- [firmware/3202087-lsc-plug/backup.sh](../../firmware/3202087-lsc-plug/backup.sh)
