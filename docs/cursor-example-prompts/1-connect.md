# Step 1: Connect the device

**In chat:** `@1-connect` model name or SKU or foldername

**Prerequisite:** step 0 done. Folder from `0-feasibility.md` or **In chat**. No SKU re-ask.

## Goal

A safe debug access path is documented: how to open the case (if needed), correct wiring or USB path, boot/flash entry for this chip family, and a checklist to confirm a working serial link, incl the cli (example `/dev/cu.usbserial-AG0KXO8J`). The user can see boot text, a ROM banner, or intentional stock output (not garbage or silence).

## Sources

- `docs/<modelnumber>-<short-name>/0-feasibility.md` (chip family, access type, verdict are settled)
- Matching device docs in this repo under `docs/<modelnumber>-<short-name>/` (e.g. `mac-host-setup.md`, `sources.md`)
- User: photos of their module, adapter, and wiring; port name after plug-in
- External research only for [to be verified] gaps listed in `0-feasibility.md`. Do not redo feasibility from scratch



## Rules

- Stop before any firmware read, write, erase, or flash
- No mains power during wiring or UART work on mains devices. Bench work at 3.3 V logic unless evidence says otherwise
- Access follows `0-feasibility.md`:
  - **Native USB** (e.g. ESP32-S3 dev board): driver and port first; UART pads optional
  - **UART pads** (typical Beken/Tuya mains): USB-TTL at 3.3 V; user solders pads
  - **SWD/JTAG:** only if documented for this device; do not guess pinout
- Boot/flash entry varies by family. Use repo docs and `0-feasibility.md`; do not assume Espressif wiring on a Beken board
- Cite URL, repo path, or photo filename; else [to be verified]. Never invent pad labels or voltages
- Never write under `notes/`
- Ask exactly one focused question only if blocked; otherwise proceed



## Deliverable

Save `docs/<modelnumber>-<short-name>/1-connect.md` with:

- **Step 1: Gaps only:** [to be verified] items still open from `0-feasibility.md`
- **Step 2: Wiring and opening:** case opening; wiring table (pad or USB → adapter/host pin, logic voltage, boot/flash pin state); warnings for this chip family and device type (mains, capacitors, sealed cases)
- **Step 3: Link verification:** port-list commands (macOS / Linux / Windows); baud and settings to try first; checklist (boot text vs garbage vs silence, TX/RX swap, boot pin retry, driver install); pitfalls before any dumo



## Next

After the user confirms the serial link works: `@2-dump-firmware`.