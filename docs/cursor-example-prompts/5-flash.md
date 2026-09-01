# Step 5: Flash custom firmware

**In chat:** `@5-flash` `/dev/cu.wchusbserial1120`

**Prerequisite:** step 4 done. Artifact in `4-write-firmware.md`. Factory backup on disk, untouched.

## Goal

Approved custom firmware is flashed to the device and verified running. Flash session is logged in the repo for rollback reference.

## Sources

- `docs/<modelnumber>-<short-name>/0-feasibility.md` (chip family, access type, flash tool)
- `docs/<modelnumber>-<short-name>/4-write-firmware.md` (artifact path, build notes)
- `docs/<modelnumber>-<short-name>/1-connect.md` (port, boot procedure)
- Custom firmware project under `firmware/<modelnumber>-<short-name>/`
- Factory backup file in the same folder (verify presence; never modify)

User: explicit "go" or "run it" after reviewing flash commands; post-flash observations.

## Rules

- This is the only step where writing to the chip is allowed
- Never erase or overwrite the factory backup file
- Stop if serial port is ambiguous or held by another process
- Stop if the command would wipe partitions or data not backed up
- Show exact flash commands (rebuild first if step 4 artifact is missing or stale). Wait for explicit user confirmation before running
- Tool chain must match chip family from `0-feasibility.md` (e.g. ESP-IDF + `idf.py flash` for Espressif; `ltchiptool` for Beken; not interchangeable)
- Never write under `notes/`
- Do not replan firmware, redo dump, or reopen feasibility

## Deliverable

Save `docs/<modelnumber>-<short-name>/5-flash.md` with:

- **Step 1: Serial port check:** port-list commands; confirm port is free. Note if stopped
- **Step 2: Project and target confirmation:** exact project path and flash target. Note if stopped
- **Step 3: Flash command:** exact, complete command(s) to flash only the approved firmware. Mark whether user confirmed before run
- **Step 4: Post-flash verification:** serial monitor baud and port (or USB log); first lines or on-device behavior that prove custom firmware is running

## Next

None. Custom firmware is running or rollback is documented in `5-flash.md` and the factory backup under `firmware/<modelnumber>-<short-name>/`. Start a new firmware iteration only if the user asks.
