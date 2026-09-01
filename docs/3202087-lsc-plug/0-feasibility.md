# LSC Smart Connect 3202087.2: feasibility

**Device folder:** `3202087-lsc-plug`

Action/LSC EU smart plug, article **3202087.2**. UART solder + Beken flash workflow; not ESP8266/`esptool`.

## Hardware

| Item | Detail |
| --- | --- |
| Product | LSC Smart Connect intelligenter Stecker |
| Article | 3202087.2 |
| Radio | 2.4 GHz Wi‑Fi |
| Load | Mains relay plug; bench work uses **3.3 V** only (no **230 V**) |
| Module (documented unit) | Tuya **T1-2S-NL** on blue daughterboard |
| SoC (documented unit) | Beken **BK7238** family via LibreTiny [ESPHome Devices] |
| Older same-line reports | BK7231N / BK7231T on WB2S [Keet Support, Elektroda], confirm marking on your module |

## Verdict

| Item | Value |
| --- | --- |
| Reprogrammable | **Yes** (community-confirmed on this article line) |
| Chip family | Beken BK72xx |
| Access type | UART pads on blue module (solder required) |
| Dump / flash tool | **ltchiptool** (not `esptool`) |
| Cloudcutter | Reported **does not** work on this generation |

## Reprogram paths

| Path | Tool | Notes |
| --- | --- | --- |
| Full flash read / write | `ltchiptool` | `./firmware/3202087-lsc-plug/backup.sh` for read |
| Replacement firmware | OpenBeken, ESPHome, LibreTiny | GPIO map below |
| Wrong-family tool | `esptool` | Do not use |

Baud **115200**. Boot/flash entry: pulse **CEN** on the **back edge** of the blue module to **GND** ~0.25 s when the tool links. Wiring: [1-connect.md](1-connect.md).

## GPIO map (ESPHome Devices, documented 3202087.2)

| Pin | PCB / function |
| --- | --- |
| P24 (K1) | Relay |
| P26 (S1) | Front button |
| P9 (D-K) | Relay LED |
| P11 (D-WIFI / TX1) | Wi‑Fi status LED |
| P1 | BL0937 SEL |
| P6 | BL0937 CF |
| P8 | BL0937 CF1 |

Energy monitoring via **BL0937** on units that include it [ESPHome Devices].

On the green mains PCB, **D-WIFI** and **D-K** are **GPIO/LED lines** (P11, P9), not serial. UART is on the **blue module** only [ESPHome Devices].

## Stock firmware (factory `.bin`)

Dump on disk (2026-08-28): `firmware/3202087-lsc-plug/factory-20260828-2101-bk7238.bin`, Tuya **T1** stack on **BK7238** (strings: `T1_2.0.0`, `beta.18_T1_wifi_ble_com_0.0.4`). Relay, BL0937 energy monitor, Wi‑Fi/BLE pairing, MQTT/TLS to `h3.iot-dns.com`, OTA via `diff2ya` patches.

The image is **not** source you can edit. Keep it for rollback. Session log: [factory-firmware-pull.md](factory-firmware-pull.md).

## References

- [1-connect.md](1-connect.md), UART wiring and link check
- [sources.md](sources.md), external guides
