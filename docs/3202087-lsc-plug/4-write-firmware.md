# LSC Smart Connect 3202087.2: write firmware

**Device folder:** `3202087-lsc-plug`

Prerequisite: [3-decide-changes.md](3-decide-changes.md). Factory backup verified in `firmware/3202087-lsc-plug/`.

**Status:** **Morse SOS demo firmware running** on P9 (D-K relay LED) via LibreTiny Arduino ([firmware/3202087-lsc-plug/morse-sos/](../../firmware/3202087-lsc-plug/morse-sos/)). Factory stock restored first, then Morse UF2 flashed 2026-08-30 ([5-flash.md](5-flash.md)). No Wi-Fi, Bluetooth, MQTT, or relay control. Factory `.bin` unchanged on disk for rollback.

Prior OpenBeken flash did not stick; leftover SOS on the status LED was cleared by factory restore before the Morse write.

## Goal (demo path, achieved)

Local replacement firmware with no cloud: Morse SOS on the relay LED (GPIO P9). Demonstrates connect → dump → decide → write firmware → flash on owned Beken hardware.

## Target features (future, not in Morse demo)

1. **Dynamic Tariff and Solar Matching**: schedule relay on/off from local price or surplus solar signals.
2. **Standby Power Auto-Cut**: cut phantom load when standby draw stays below a threshold.
3. **Turn LED light off**: disable relay and Wi‑Fi status LEDs.

## Build path (Morse demo)

| Item | Choice |
| --- | --- |
| Platform | LibreTiny Arduino |
| Board | `t1-2s` (Tuya T1-2S-NL, BK7238) |
| GPIO | P9 relay LED only; P11 and P24 unused |
| Flash artifact | `firmware.uf2` via `ltchiptool flash write` (not `pio upload`) |
| Flash tool | `ltchiptool` (not `esptool`) |
| Rollback | Write verified factory `.bin` from [2-dump-firmware.md](2-dump-firmware.md) |

## Files

- [firmware/3202087-lsc-plug/morse-sos/](../../firmware/3202087-lsc-plug/morse-sos/), Morse SOS source and PlatformIO project
- [5-flash.md](5-flash.md), commands and session log

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| Brick during flash | Verified factory `.bin` on disk; CEN pulse procedure in [1-connect.md](1-connect.md) |
| Wrong GPIO map | P9 only per [0-feasibility.md](0-feasibility.md); P11/P24 not used |
| Mains safety | **No 230 V** during UART work |
| Wrong image type | Factory: raw 2 MiB with `-f bk7238 -s 0x0 -l 0x200000`; Morse: `firmware.uf2` auto-detect |

## Success criteria

- [x] Factory dump 2097152 bytes, SHA-256 recorded
- [x] Replacement firmware builds locally (`morse-sos`)
- [x] GPIO map matches documented 3202087.2 unit (P9)
- [x] Flash recorded in [5-flash.md](5-flash.md)

## Rollback

```bash
.venv/bin/ltchiptool flash write -d /dev/cu.usbserial-PORT -f bk7238 -s 0x0 -l 0x200000 \
  firmware/3202087-lsc-plug/factory-20260828-2101-bk7238.bin
```

Same **CEN** pulse on connect. See [5-flash.md](5-flash.md).
