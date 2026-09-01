# Step 4: Write and build custom firmware

**In chat:** `@4-write-firmware` model name or SKU or foldername

**Prerequisite:** step 3 done. Direction from `3-decide-changes.md`

## Goal

The chosen custom firmware change is planned, implemented in the repo tree, and built locally. A flash-ready artifact path is recorded in the repo. Factory backup remains untouched.

## Sources

- `docs/<modelnumber>-<short-name>/3-decide-changes.md` (chosen direction)
- `0-feasibility.md`, `1-connect.md`, `2-dump-firmware.md`
- Factory dump under `firmware/<modelnumber>-<short-name>/` (read-only reference; do not modify)
- Existing custom firmware tree under `firmware/<modelnumber>-<short-name>/` if present
- Family toolchain docs (ESP-IDF, LibreTiny, OpenBeken, vendor SDK) cited in device docs

External research only for [to be verified] facts needed to implement (e.g. GPIO map, SDK version). Do not redo decide, dump, or connect steps.

## Rules

- Stay on this device, chip family, and access type only
- Local compile and debug are in scope. Never flash, erase, or write to the chip
- Never overwrite the factory `.bin`. Never write under `notes/`
- Build path follows chip family (e.g. ESP-IDF for Espressif; LibreTiny or OpenBeken for Beken). Mark missing facts [to be verified]. Ask exactly one clarifying question if a critical fact blocks implementation
- Demo path: no Wi-Fi, Bluetooth, or cloud APIs unless `3-decide-changes.md` explicitly requires them
- Do not invent GPIO maps. Prefer repo sources and documented maps from step 0

## Deliverable

Save in the repo:

1. Firmware project under `firmware/<modelnumber>-<short-name>/` (create or extend per plan)
2. `docs/<modelnumber>-<short-name>/4-write-firmware.md` with:
   - **Step 1: Clarification:** [to be verified] items or exactly one question; else "No clarifying question needed"
   - **Step 2: Concrete plan:** goal (one sentence); files to touch; risks and mitigations; success criteria; rollback; first implementation step (nothing that writes to the chip)
   - **Step 3: Implement custom firmware:** what was created or edited
   - **Step 4: Local build:** build command(s), result, and artifact path (e.g. `.bin`, `.uf2`)

## Next

After the user confirms the build artifact is ready: `@5-flash`.
