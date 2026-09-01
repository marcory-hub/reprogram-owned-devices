# LSC Smart Connect 3202087.2: decide changes

**Device folder:** `3202087-lsc-plug`

Prerequisite: verified factory dump in [2-dump-firmware.md](2-dump-firmware.md).

## Step 1: Stock firmware summary

The backed-up image is a **raw 2 MiB flash** (machine code + Tuya data), not editable source.

| Area | Stock behavior |
| --- | --- |
| Cloud | MQTT/TLS to `h3.iot-dns.com`; app pairing over Wi‑Fi/BLE |
| Actuators | Relay (P24), front button (P26), status LEDs |
| Sensing | BL0937 energy monitor on documented units |
| OTA | Tuya `diff2ya` patch updates |
| Local control | Via vendor app / cloud only |

SDK strings (2026-08-28 dump): Tuya **T1** `T1_2.0.0`, `beta.18_T1_wifi_ble_com_0.0.4`, `tuyaos-iot_3.8.31`.

Deep static analysis: [disassembly.md](disassembly.md).

## Step 2: Demo choice (implemented)

For this repo’s Cursor demo, the plug runs **Morse SOS** (`... --- ...`) on **GPIO P9** (D-K relay LED) only:

| Item | Choice |
| --- | --- |
| Firmware | [firmware/3202087-lsc-plug/morse-sos/](../../firmware/3202087-lsc-plug/morse-sos/) (LibreTiny Arduino, board `t1-2s`) |
| Scope | No Wi-Fi, Bluetooth, MQTT, relay, or cloud |
| Why P9 | Relay LED per [0-feasibility.md](0-feasibility.md); P11 (status LED) and P24 (relay) unused |
| Flash | Restore factory `.bin` first, then `firmware.uf2` via `ltchiptool` ([5-flash.md](5-flash.md)) |

This proves connect → dump → decide → write firmware → flash on owned Beken hardware without a vendor account.

## Step 3: Future directions (not in Morse demo)

1. **Drop cloud dependency**: OpenBeken or ESPHome with local MQTT/Home Assistant; no Tuya account.
2. **Local energy logging**: expose BL0937 readings on LAN instead of cloud-only history.
3. **Relay automation**: time-of-use rules, solar surplus matching, standby cut (see [4-write-firmware.md](4-write-firmware.md)).
4. **Disable status LED**: turn off relay/Wi‑Fi LEDs for bedroom plugs.
5. **Security posture**: replace stock Tuya stack; keep factory `.bin` for rollback only.

## Step 4: Critical ranking (future work)

| Direction | Payoff | Risk | Reversibility |
| --- | --- | --- | --- |
| Cloud removal | High | Medium (flash) | High if factory `.bin` kept |
| Energy logging local | High | Low | High |
| Relay automation | High | Medium | High |
| LED off | Medium | Low | High |
| Full RE audit of stock `.bin` | Low for demo | High effort | N/A |

**Practical path for the demo:** keep factory `.bin` for restore; flash Morse SOS ([5-flash.md](5-flash.md)). Larger local-first builds can follow the directions above.

## Stock firmware audit (optional)

You **cannot** review the factory image like normal source. Bug checking is reverse engineering or black-box testing.

| Approach | Effort | Outcome |
| --- | --- | --- |
| Static strings scan | Low | SDK version, cloud URLs, hints only |
| Behavioral test (stock FW) | Medium | Wi‑Fi/BLE traffic, OTA behavior |
| Disassembly (Ghidra) | High | Specialist work |
| Flash replacement FW | Medium | Removes most stock cloud risk |

Quick static pass (2026-08-28): standard Tuya IoT SDK; TLS/AES present; **Cloudcutter** does not apply ([sources.md](sources.md)).

## References

- [0-feasibility.md](0-feasibility.md), GPIO map
- [5-flash.md](5-flash.md), Morse SOS flash procedure
- [disassembly.md](disassembly.md), partition and strings analysis
- [factory-firmware-pull.md](factory-firmware-pull.md), dump SHA-256
