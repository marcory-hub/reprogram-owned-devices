# LSC Smart Connect UART debug report

Date: 2026-08-28  
Port: `/dev/cu.usbserial-AG0KXO8J`  
Adapter: FT232R USB UART (serial `AG0KXO8J`), jumper on 3.3 V per [docs/images/usb-uart-adapter.png](../../../docs/images/usb-uart-adapter.png)

Scope: serial link verification only. No firmware dump or flash steps.

---

## Phase 1: Electrical verification (operator)

USB adapter **unplugged from Mac** for all resistance checks below.

### 1.1 Multimeter baseline

| Check | Expected | Operator result |
| --- | --- | --- |
| Probes shorted (Ω mode) | 0-1 Ω | [ ] |
| Probes open | OL / >1 MΩ | [ ] |

Use **Ω (resistance)**, not continuity beep, for 3V3↔GND. Beep mode can misread capacitor charge as a short.

### 1.2 3V3 ↔ GND on blue daughterboard (USB unplugged, mains unplugged)

| Reading | Meaning | Action |
| --- | --- | --- |
| 0-5 Ω | Hard short | **Stop.** Do not connect VCC. Rework solder; inspect bridges between 3V3 and GND pads. |
| 50-500 Ω (may rise slowly) | Normal bulk/decoupling caps | Proceed cautiously; note value. |
| >1 kΩ or OL | No short | Safe to apply adapter VCC. |

**Prior session:** continuity showed 000 (hard short) after soldering. That condition must be cleared and re-measured here before Phase 2 power-on.

| Measurement | Value | Pass? |
| --- | --- | --- |
| 3V3 ↔ GND (blue board, on green PCB) | ______ Ω | [ ] |
| 3V3 ↔ GND (blue board alone, if removable) | ______ Ω | [ ] |

### 1.3 Isolate fault (if still low)

- [ ] Inspect 3V3/GND pads under magnification: bridges, cold joints, splatter.
- [ ] If blue module unplugs from green mains PCB, measure 3V3↔GND on module only.
  - Short on module alone → blue module damaged or bad solder.
  - Short only when mounted → green PCB or connector fault.

### 1.4 Capacitor vs hard short

- Hard short: stable **0-5 Ω** in both directions, instant on beep.
- Capacitor: resistance **starts low and climbs** over seconds; beep is brief.

### Phase 1 stop: do not proceed if

- 3V3↔GND stays 0-5 Ω after rework
- Burn smell, charred pad/trace, or cracked module
- Pad labels (3V3, GND, RX1, TX1) uncertain
- Stekker plugged into 230 V wall socket

---

## Phase 2: Adapter and wiring

### 2.1 Adapter health (no target wires)

Host check **2026-08-28:** `/dev/cu.usbserial-AG0KXO8J` present; ioreg shows `FT232R USB UART`, serial `AG0KXO8J`.

Operator with **target wires disconnected**:

| Signal | Expected |
| --- | --- |
| Power LED | One LED on |
| TX/RX LEDs | Off (idle) |
| Adapter temperature | Cool |

If multiple LEDs stay on or adapter runs hot with no target → wiring fault or adapter damage. Unplug USB.

### 2.2 Required 4-wire map

Adapter header (top → bottom on [usb-uart-adapter.png](../../../docs/images/usb-uart-adapter.png)): **DTR, RXI, TXO, VCC, CTS, GND**.

| Adapter pin | Target pad | Notes |
| --- | --- | --- |
| GND | GND | |
| VCC (3.3 V jumper) | 3V3 | Never CTS. Prior mistake: brown on CTS. |
| TXO | RX1 | Adapter TX → target RX |
| RXI | TX1 | Adapter RX ← target TX |
| DTR |, | Leave empty |
| CTS |, | Leave empty |

### 2.3 Continuity (USB unplugged)

| Path | OK? |
| --- | --- |
| Adapter GND ↔ target GND | [ ] |
| Adapter VCC ↔ target 3V3 | [ ] |
| Adapter TXO ↔ target RX1 | [ ] |
| Adapter RXI ↔ target TX1 | [ ] |
| VCC not connected to GND | [ ] |
| TXO not connected to TX1 | [ ] |

---

## Phase 3: Serial link (host)

Script: [lsc-uart-debug.sh](lsc-uart-debug.sh)

### Commands (auto-timeout, no blocking `cat`)

List port:

```bash
ls /dev/cu.usbserial*
```

Run all baud captures (8 s each; **power-cycle target USB at start of each capture**):

```bash
./firmware/3202087-lsc-plug/uart-debug/lsc-uart-debug.sh /dev/cu.usbserial-AG0KXO8J
```

Single timed read at 115200:

```bash
stty -f /dev/cu.usbserial-AG0KXO8J 115200 cs8 -cstopb -parenb raw -echo
perl -e 'alarm 10; open F,"</dev/cu.usbserial-AG0KXO8J"; binmode F; sysread F,$b,8192; print $b' | xxd
```

74880 on macOS (`stty` rejects 74880; script uses `IOSSIOSPEED`):

```bash
./firmware/3202087-lsc-plug/uart-debug/lsc-uart-debug.sh /dev/cu.usbserial-AG0KXO8J
```

Power-cycle capture (operator unplugs/replugs USB when prompted):

```bash
./firmware/3202087-lsc-plug/uart-debug/lsc-uart-powercycle-capture.sh /dev/cu.usbserial-AG0KXO8J 115200
```

### Host run 2026-08-28 (target likely already booted; no coordinated power-cycle)

| Baud | Raw bytes | Content |
| --- | --- | --- |
| 115200 | 39 | All `0x00` |
| 74880 | 41 | All `0x00` |
| 9600 | 39 | All `0x00` |
| 38400 | 39 | All `0x00` |
| 57600 | 13 | All `0x00` |

**Earlier session (Cursor terminal):** ~15-16 bytes, printable filter showed `?%` (non-ASCII / wrong baud), not null-only.

### Pass / fail criteria

| Output | Verdict |
| --- | --- |
| ASCII boot lines (chip ID, flash size, "boot", version strings) | **Pass**: link OK |
| Short non-ASCII burst at one baud only | Wrong baud or TX/RX swap, retry swap + baud list |
| Steady `0x00` or silence | No UART traffic: power, wiring, or already past boot window |
| `?%`-style garbage at every baud, both TX/RX orientations | Signal integrity, wrong pads, or damaged module |

Chip family baud hints [to be verified]:

- ESP8266/8285 ROM often **74880** then switches
- Beken BK7231T often **115200**
- Tuya TYWE2S (ESP8285 class) often **74880** or **115200**

CEN/reset strap for Beken [to be verified]: only after chip ID confirmed from boot text or module marking.

### TX/RX swap test (if garbage/null persists after Phase 1 pass)

1. Unplug USB.
2. Swap **TXO↔RXI** at target end only (TXO→TX1, RXI→RX1 is wrong orientation).
3. Re-run `./firmware/3202087-lsc-plug/uart-debug/lsc-uart-debug.sh` with power-cycle per capture.

---

## Phase 4: Decision table

| Symptom | Likely cause | Next action |
| --- | --- | --- |
| 3V3↔GND 0-5 Ω | Solder bridge or damaged IC | Stop; rework or replace blue module |
| Adapter hot, all LEDs on | VCC into short or wrong pin | Unplug USB; fix wiring (VCC→3V3, not CTS) |
| Port missing | USB/adapter fault | Other port/cable; adapter-only test |
| Null/silence all bauds | No boot UART, wrong RX path, or missed boot window | Confirm Phase 1; power-cycle during capture; verify RXI→TX1 |
| Garbage all bauds, both swaps | Wrong pads, bad solder, damaged SoC | Microscope on pads; read chip marking [to be verified] |
| Readable text one baud | Link OK | Record baud; identify chip from log |

---

## Stop: do not dump or flash firmware if

- 3V3↔GND hard short not cleared
- Adapter overheats or multiple LEDs with no target
- No readable boot text after Phase 1 pass + both TX/RX orientations + baud list + power-cycle captures
- Intermittent link (wiggle changes output)
- Mains connected during UART work

---

## Current status

| Phase | Status |
| --- | --- |
| 1 Electrical | **Blocked pending operator Ω reading**: prior 000 short must be re-verified |
| 2 Adapter enum | **Pass**: FT232R `AG0KXO8J` on Mac |
| 2 Wiring | **Unconfirmed**: CTS→VCC fix reported; TX/RX crossover not confirmed |
| 3 Serial | **Fail**: null-only captures; earlier `?%` suggests activity at wrong baud/orientation |
| 4 Next | Operator: Phase 1 Ω → Phase 2 continuity → power-cycle capture script |

---

## References

- [README.md](../../../README.md), LSC Smart Connect section
- [docs/cursor-example-prompts/1-connect.md](../../../docs/cursor-example-prompts/1-connect.md)
- [docs/images/usb-uart-adapter.png](../../../docs/images/usb-uart-adapter.png)
