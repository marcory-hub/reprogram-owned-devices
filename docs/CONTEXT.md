# reprogram-owned-devices

Canonical terms for this Cursor demo repo. Operator procedures live under `docs/<device>/`.

## Language

**Device folder**:
Matched pair `docs/<modelnumber>-<short-name>/` and `firmware/<modelnumber>-<short-name>/` (lowercase, hyphenated; brand in slug when helpful).
_Avoid_: A second slug for the same board, generic names like `plug/` or `camera/` alone.

**Factory firmware**:
Stock flash image from the device or a verified published dump, kept before any write for rollback.
_Avoid_: Calling the `.bin` "source code"; patching the dump in place.

**Steps 0-5**:
Demo flow in README and `docs/cursor-example-prompts/`: **0** feasibility, **1** connect, **2** dump, **3** decide, **4** write firmware, **5** flash. Cursor: `@0-feasibility` through `@5-flash` with context in the same message (SKU, folder, port). Long session: `@handoff` then next skill in a fresh chat.
_Avoid_: Skipping dump before flash unless stock is already secured another way.

**Five steps**:
Steps **1-5** only (connect through flash), after step **0** feasibility.
_Avoid_: Calling feasibility one of the five; it is step 0.

**Demo path**:
Local replacement firmware with no Wi-Fi, Bluetooth, or cloud APIs on the story shown to attendees.
_Avoid_: Stock Xiaozhi / vendor cloud recognition as the demo outcome.
