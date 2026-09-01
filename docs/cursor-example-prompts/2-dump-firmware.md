# Step 2: Pull factory firmware

**In chat:** `@2-dump-firmware` `/dev/cu.usbserial-AG0KXO8J`

**Prerequisite:** steps 0-1 done. Link confirmed. Folder from `0-feasibility.md` or **In chat**.

## Goal

A verified, read-only factory firmware backup exists on disk with enough metadata to restore stock later. The immutable backup file is separate from any custom firmware work.

## Sources

- `docs/<modelnumber>-<short-name>/0-feasibility.md` (chip family and dump tool)
- `docs/<modelnumber>-<short-name>/1-connect.md` (port, baud, boot procedure)
- `firmware/<modelnumber>-<short-name>/` in this repo (reuse `backup.sh` if present)
- User: confirmed serial port name, dump result (file size, hash if available)

External research only for [to be verified] gaps that block the read command. Do not replan connect or feasibility.

## Rules

- Read-only only. Never suggest write, erase, or flash commands
- Tool must match chip family from `0-feasibility.md` (e.g. Espressif → `esptool read_flash`; Beken → `ltchiptool` read/backup; STM32 → documented read path). Do not use `esptool` on Beken
- Never overwrite a verified factory `.bin` once written. Use a new dated filename if re-dumping
- Never write under `notes/`
- Do not propose custom firmware, compile steps, or flash until the user confirms the dump is verified

## Deliverable

Save in the repo:

1. Factory binary under `firmware/<modelnumber>-<short-name>/` (descriptive, dated filename OK)
2. `docs/<modelnumber>-<short-name>/2-dump-firmware.md` with:
   - **Step 1: Serial port:** port-list commands; how to confirm the correct port
   - **Step 2: Safe full-firmware dump:** exact read-only command(s); baud, flash size, chip-specific flags; how to verify success (file size, hash, expected byte count)
   - **Step 3: File organization and restore notes:** filenames and paths; flash size, address/length, chip type, port, tool flags, date, restore procedure. Note if `backup.sh` was used
   - **Step 4: Safety checklist:** wiring, boot pin, voltage, port, mains off, backup untouched; success vs common failure signs

## Next

After the user confirms the dump is verified: `@3-decide-changes`.
