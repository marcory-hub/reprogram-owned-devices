# Step 3: Decide what to build

**In chat:** `@3-decide-changes` model name or SKU or foldername - optional: ideas for new firmware

**Prerequisite:** step 2 done. Folder from prior docs. No SKU re-ask.

## Goal

Stock firmware behavior is summarized from repo evidence. Three to five concrete, local-first custom firmware directions are proposed and ranked so the user can pick one for implementation.

## Sources

- `docs/<modelnumber>-<short-name>/0-feasibility.md`, `1-connect.md`, `2-dump-firmware.md`
- Factory binaries under `firmware/<modelnumber>-<short-name>/` (strings, headers, known stock behavior documented in repo)
- Other repo docs for this device (e.g. `sources.md`, session logs)
- User: priorities (privacy, sensors, display, automation, fun demo)

External research only to fill [to be verified] gaps about stock behavior not documented in the repo. Do not redo feasibility, wiring, or dump procedures.

## Rules

- Stop before compile, flash, erase, write to the chip, or source code implementation
- Use only information in this repo plus cited external gaps. Cite file paths. Mark gaps [to be verified]
- Do not invent tear-downs, schematics, pinouts, or undocumented features
- Prefer solutions that reduce or eliminate cloud dependence
- Match proposals to chip family and device type from `0-feasibility.md`
- Never write under `notes/`

## Deliverable

Save `docs/<modelnumber>-<short-name>/3-decide-changes.md` with:

- **Step 1: Stock firmware summary:** cloud, local control, sensors, actuators, display/LEDs, radio, OTA/pairing, safety or privacy issues. Factual only; cite repo sources
- **Step 2: Local-first improvement directions:** 3-5 concrete directions. For each: problem solved, technical approach, expected user benefit
- **Step 3: Critical ranking:** rank by payoff vs risk (user benefit, cloud reduction, difficulty, brick risk, reversibility). Prefer lower cloud dependence when scores are close

## Next

After the user picks a direction (or confirms the top-ranked pick): `@4-write-firmware`.
