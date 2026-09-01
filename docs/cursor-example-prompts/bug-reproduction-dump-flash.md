# Bug reproduction: UART, dump, and flash

**In chat:** `@bug-dump-flash` dump timeout `/dev/cu.usbserial-*` Getting bus failed

**When to use:** dump (2), flash (5), or UART (1). Not step 0, 3, or 4 build errors.

**Device:** [SKU or folder slug]

**Step stuck on:** dump (2) / flash (5) / connect (1) underpinning dump or flash

**Symptoms:** [timeout, garbage, silence, partial file, wrong tool error, port missing, …]

**Optional paste:** excerpts from `0-feasibility.md`, `1-connect.md`, port name, exact command, error text, photo of wiring

## Goal

Confirm whether the failure is UART/link related (most common) or tool/command related. Produce a minimal, safe reproduction sequence the operator can run locally. Rank likely causes. Stop before any destructive chip write unless a verified factory backup is already on disk.

## Sources

- User: symptoms, step number, port name, exact command and full error text, OS, adapter model, wiring photo description
- User paste: `0-feasibility.md` (chip family, access type, dump/flash tool), `1-connect.md`, `2-dump-firmware.md` or `5-flash.md` if they exist
- This repo (if user pastes or names paths): demo failure tables in `docs/3202087-lsc-plug/1-connect.md`, `2-dump-firmware.md`, `docs/SAD00006D-ai-cam/mac-host-setup.md`, `firmware/3202087-lsc-plug/uart-debug/lsc-uart-debug-report.md`
- External research only for [to be verified] gaps. Do not invent pad labels or pinouts

## Rules

- Read-only diagnostics first (port list, loopback, resistance check, `read_flash` only when dump is the goal and user already intended read)
- Never suggest `write_flash`, erase, or flash until:
  - dump failures are ruled out as link/tool issues, and
  - for flash-only bugs: user confirms a verified factory backup exists on disk
- Tool must match chip family from evidence (`esptool` / ESP-IDF for Espressif; `ltchiptool` for Beken). Flag wrong-family errors immediately
- One focused question if blocked (e.g. loopback result, 3V3↔GND Ω reading, exact port string). Otherwise proceed
- Do not redo full feasibility or firmware design
- Mains devices: **no 230 V** during UART work. Bench at **3.3 V** logic unless evidence says otherwise
- Missing serial boot text is **normal** on some Beken/Tuya plugs if power and loopback pass. Do not treat silence alone as bad wiring
- Never write under `notes/` (Cursor-only paths; Grok returns markdown only)

## Deliverable

Return this bug report in chat (user pastes into Cursor):

```markdown
# Bug report: [device folder] / [dump / flash / UART]

## Environment
- OS:
- Step stuck on:
- Chip family (from feasibility):
- Tool used:
- Port:
- Command (full):

## Symptom
- Expected:
- Actual:
- Error text (exact):

## Minimal reproduction
1. [Safe step, e.g. list ports, loopback, resistance check]
2. …
3. [Stop point before any write if backup not verified]

## UART link checklist (mark done / fail / not tried)
- [ ] Correct port (only one adapter plugged in)
- [ ] TX/RX crossed (adapter TX → target RX, adapter RX → target TX)
- [ ] 3.3 V logic (not 5 V on target)
- [ ] 3V3 ↔ GND not a hard short (Ω mode, not beep)
- [ ] Boot/download entry (ESP: BOOT+RESET; Beken: CEN pulse to GND ~0.25 s)
- [ ] Data-capable USB cable (native USB devices)
- [ ] Driver installed (e.g. CH340 on macOS camera)
- [ ] Adapter loopback PASS

## Ranked hypotheses
| Rank | Hypothesis | Evidence | Next safe test |
| --- | --- | --- | --- |
| 1 | … | … | … |

## Symptom quick map (use matching rows only)
| Symptom | Likely UART/link cause | Likely tool/command cause |
| --- | --- | --- |
| Port missing | Driver, cable, wrong USB port | n/a |
| Timeout / "Getting bus failed" | Wrong port, TX/RX swap, no boot pulse, connect lost | Wrong chip family flag |
| Garbage at all bauds | TX/RX swap, wrong voltage, noisy ground | Wrong baud for family |
| Silence (no text) | Often **normal** on Beken if power OK; else wiring | n/a |
| Partial dump file | Link dropped mid-read | Wrong flash size; delete partial and retry |
| `esptool` errors on Beken chip | n/a | Wrong tool; use `ltchiptool` |
| CRC mismatch at start (Beken) | Link glitch | Try `bk7238` vs `bk7231n` |
| Flash cannot connect (ESP USB) | Charge-only cable, need BOOT+RESET | `idf.py` env not sourced |
| Flash OK but blank LCD | n/a | Press RESET; wrong partition layout vs stock |

## Hand off to Cursor
- Fix category: wiring / driver / boot procedure / tool family / command flags / not UART (build artifact)
- Suggested Cursor skill: `@1-connect`, `@2-dump-firmware`, or `@5-flash` (match fix category)
- Files to update if repro confirms root cause: …
```

## Next

Paste the bug report into Cursor. Then invoke the matching skill:

- Wiring, driver, or boot procedure: `@1-connect`
- Dump command or read failure: `@2-dump-firmware`
- Flash command or post-flash behavior: `@5-flash`

If UART is fixed but dump or flash still fails: `@bug-dump-flash` again with updated symptoms.
