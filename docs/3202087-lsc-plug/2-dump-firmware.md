# LSC Smart Connect 3202087.2: dump firmware

**Device folder:** `3202087-lsc-plug`

Read-only full-flash dump before any custom write. **Do not** use `esptool`.

Prerequisite: [1-connect.md](1-connect.md) (debug link confirmed).

## What the `.bin` is (and is not)

The pull saves a **raw 2 MiB flash image** (`factory-*.bin` under `firmware/3202087-lsc-plug/`). That file is machine code plus Tuya data stored on the chip. It is **not** source code you can open, edit, or step through like a normal project.

| You can | You cannot |
| --- | --- |
| Keep it as a **rollback** copy of stock | Read it like firmware source in Cursor |
| Check **size** (2097152 bytes) and **SHA-256** | Rebuild or patch behavior by editing the `.bin` |
| **Write it back** with ltchiptool to restore stock | Use `esptool` (wrong chip family) |

To change what the plug does, flash **replacement** firmware (OpenBeken, ESPHome, LibreTiny) using the GPIO map in [0-feasibility.md](0-feasibility.md). Session log: [factory-firmware-pull.md](factory-firmware-pull.md).

## Prerequisites

- **No 230 V** on the plug; module powered from USB-TTL **3.3 V** only.
- FT232RL (or similar) on **3.3 V**; TX/RX crossed to module **RX1** / **TX1**.
- **CEN** wire on **Side B** (back edge) of the blue module. See [1-connect.md](1-connect.md).
- Module marking confirmed if possible (BK7238 vs BK7231N/T). Documented 3202087.2 unit: **BK7238** [ESPHome Devices]; older batches: BK7231N [Keet Support].

## 1. Serial port

```bash
ls /dev/cu.usbserial*
```

Example from operator session: `/dev/cu.usbserial-AG0KXO8J`. Pick the port that appears when the adapter is plugged in.

## 2. Install ltchiptool (once per machine)

From repo root:

```bash
python3 -m venv .venv
.venv/bin/pip install -r requirements/ltchiptool.txt
.venv/bin/ltchiptool list families | grep -i bk723
```

Supported families include `bk7238`, `bk7231n`, `bk7231t`.

## 3. Full flash read (read-only)

Default flash size for this family: **2 MiB** (2097152 bytes). Baud for link: **115200**.

### Option A: helper script

```bash
./firmware/3202087-lsc-plug/backup.sh
```

Override port or family if needed:

```bash
PORT=/dev/cu.usbserial-AG0KXO8J FAMILY=bk7238 ./firmware/3202087-lsc-plug/backup.sh
```

### Option B: manual command

```bash
OUT="firmware/3202087-lsc-plug/factory-$(date +%Y%m%d-%H%M)-bk7238.bin"
.venv/bin/ltchiptool flash read -d /dev/cu.usbserial-AG0KXO8J bk7238 "$OUT"
```

If link fails with CRC or timeout on **bk7238**, retry with `bk7231n` (same wiring).

## 4. Enter download mode (CEN)

**CEN** is on **Side B**, the **back edge** of the blue **T1-2S-NL** module, opposite 3V3/GND/RX1/TX1. It is **not** on the green mains PCB. See [1-connect.md](1-connect.md).

When ltchiptool shows connection instructions or **"Getting bus failed"** / timeout:

1. Keep the command running (or restart it).
2. Pulse **CEN → GND** for about **0.25 s** (tap a loose wire).
3. Alternatively power-cycle: disconnect adapter **VCC** briefly, reconnect, then pulse CEN during the connect window.

Do **not** leave CEN shorted to GND permanently.

Success log lines look like:

```text
Success! Chip info: ...
Reading Flash (2 MiB) to '...'
[============================================================] 100%
```

## 5. Verify dump

```bash
ls -l firmware/3202087-lsc-plug/factory-*.bin
```

| Check | Expected |
| --- | --- |
| File size | **2097152** bytes (2 MiB) |
| Partial file | Delete and re-run if size &lt; 2097152 |

```bash
wc -c firmware/3202087-lsc-plug/factory-*.bin
shasum -a 256 firmware/3202087-lsc-plug/factory-*.bin
```

Record SHA-256 in [factory-firmware-pull.md](factory-firmware-pull.md) if you keep multiple dumps.

Optional type check:

```bash
.venv/bin/ltchiptool flash file firmware/3202087-lsc-plug/factory-YYYYMMDD-HHMM-bk7238.bin
```

## 6. Restore later (reference only)

Restore uses **write**, not read. Only after backup is verified. See [5-flash.md](5-flash.md).

## Safety checklist

- [ ] **No 230 V** on plug (adapter **3.3 V** only)
- [ ] 3V3 ↔ GND not a hard short (kΩ range OK from prior session)
- [ ] Correct `/dev/cu.usbserial*` (only one adapter plugged in)
- [ ] CEN wire on blue module Side B
- [ ] Output path under `firmware/3202087-lsc-plug/`
- [ ] Dump size exactly **2097152** bytes before any flash write

## Failure signs

| Symptom | Likely cause |
| --- | --- |
| Timeout / "Getting bus failed" | No CEN pulse, wrong port, or TX/RX swapped |
| Progress stops below 2 MiB | Connect lost; delete partial `.bin` and retry |
| CRC mismatch at start | Wrong family (`bk7238` vs `bk7231n`); try the other |
| `esptool` errors | Wrong tool family (chip is Beken) |

## References

- [1-connect.md](1-connect.md)
- [factory-firmware-pull.md](factory-firmware-pull.md)
- [sources.md](sources.md)
- [firmware/3202087-lsc-plug/backup.sh](../../firmware/3202087-lsc-plug/backup.sh)
- [LibreTiny ltchiptool](https://docs.libretiny.eu/docs/flashing/tools/ltchiptool/)
