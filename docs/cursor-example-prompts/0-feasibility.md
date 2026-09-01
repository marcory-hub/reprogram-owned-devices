# Step 0: Check reprogram feasibility

**In chat:** `@0-feasibility` model name or SKU or foldername

**Optional:** sticker model, FCC ID, region, PCB or label photos

## Goal

Go or no-go on whether this device can run local replacement firmware. A device folder name is chosen. Chip family, access type, dump/flash tool family, and reprogram paths are documented from evidence. Open gaps are listed for the connect step.

## Sources

- User: product name, exact article or SKU, optional photos of chip markings and labels
- This repo: `README.md`, `docs/`, `firmware/`, `.cursor/rules/project-map.mdc`
- External (cite URL or mark [to be verified]): vendor wikis, ESPHome Devices, OpenBeken, Tuya Cloudcutter (Espressif-based Tuya only, not BK72xx/BK7238), community tear-downs and flash guides

Research to resolve [to be verified] is allowed in this step.

## Rules

- Stop before wiring, factory dump, custom firmware, or flash
- Exact article or SKU before treating siblings as evidence. Tag sibling inferences as [sibling SKU, not confirmed]
- Photos of chip markings override web guesses. Never invent pinouts, pad names, port commands, or success stories
- Same vendor or app does not imply the same chip. Name the family from evidence only
- Never write under `notes/`
- Wrong-family tools (e.g. `esptool` on Beken): note once in blockers. Later steps trust this file
- Reuse an existing device folder if present (e.g. `SAD00006D-ai-cam`, `3202087-lsc-plug`). Do not invent a second slug



## Deliverable

Save in the repo:

1. `docs/<modelnumber>-<short-name>/0-feasibility.md` with **Device folder:** at the top
2. `firmware/<modelnumber>-<short-name>/` (empty folder OK; no factory `.bin` yet)

File sections (use these `##` headings):

- **Step 1: Exact-documentation:** tear-downs, flash guides, Cloudcutter profiles where applicable; user photos (filename and what each proves); gaps.
- **Step 2: Sibling cross-check:** related documented models with article, chip, flash path; tag inferences.
- **Step 3: Chip and platform:** confirmed or likely chip/module family; device type; stock platform; power; blockers. Access type if known: native USB, UART pads, or other. No wiring yet.
- **Step 4: Reprogram paths:** only paths that match the chip family. Each: evidence, tool name, main risks. No pad names or port commands
- **Step 5: Verdict:** device folder; reprogrammable (yes / probably / unknown / unlikely) and why; confirmed chip family, access type, dump/flash tool; open [to be verified] for connect; `@1-connect` if continuing



## Next

- Run `@critical` on saved `0-feasibility.md` and any feasibility claims still only in chat
- For each issue `@critical` reports: edit `0-feasibility.md` (wrong then fix or delete; hype then soften; no source then cite vendor docs or this repo, or mark [to be verified])
- Verdict yes, probably, or unknown: `@1-connect`

