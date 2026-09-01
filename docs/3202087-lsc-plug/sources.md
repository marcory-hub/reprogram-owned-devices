# LSC Smart Connect 3202087.2: sources

Curated references for the Action/LSC smart plug (article **3202087.2**). Prefer these over random tear-downs; Action may change the module inside without changing the box.

## Product and retail

| Source | URL | What it shows |
| --- | --- | --- |
| Action product page (DE) | https://www.action.com/de-de/p/3202087/lsc-smart-connect-intelligenter-stecker/ | Retail SKU, product name, packaging photo |
| ESPHome Devices, LSC plug 3202087.2 | https://devices.esphome.io/devices/lsc-plug-32020872/ | Module photos, UART pad labels, GPIO map, ESPHome/LibreTiny config, BK7238 notes |

## Flashing and UART guides

| Source | URL | What it shows |
| --- | --- | --- |
| Keet Support, flash LSC with ESPHome (2024, updated 2025) | https://keetsupport.nl/2024/03/20/how-to-flash-lsc-power-plug-with-esphome/ | BK7231N on article 3202087, UART wiring, **CEN** pulse for flash mode, ltchiptool workflow |
| OpenBK LSC Action1681PG (GitHub) | https://github.com/do8pgg/OpenBK_LSC_Action1681PG | German guide: RX1/TX1 crossover table, OpenBeken backup/flash, “Getting bus failed” + GND reset |
| Elektroda, LSC plug 2578685 (BK7231T, no energy meter) | https://www.elektroda.com/rtvforum/topic3944507.html | Torx opening, WB2S/BK7231T variant, OpenBeken flash |
| Elektroda, WB2S Action smart plug with power monitoring | https://www.elektroda.com/rtvforum/topic4038087.html | WB2S pinout table, 3.3 V TTL, CEN pulse, CH340/CP2102 tested |

## Vendor / module datasheets

| Source | URL | What it shows |
| --- | --- | --- |
| Tuya WB2S module datasheet | https://developer.tuya.com/en/docs/iot/wb2s-module-datasheet?id=K9ghecl7kc479 | WB2S pin functions (1RX, 1TX, CEN, VBAT 3.3 V), use when module marking matches WB2S [to be verified per unit] |

## In-repo assets (this project)

| Path | What it shows |
| --- | --- |
| [0-feasibility.md](0-feasibility.md) | Chip, GPIO, verdict |
| [1-connect.md](1-connect.md) | UART wiring, CEN on blue module back edge |
| [2-dump-firmware.md](2-dump-firmware.md) | ltchiptool read-only pull and verification |
| [4-write-firmware.md](4-write-firmware.md) | Custom firmware plan, build path, and Morse SOS demo |
| [5-flash.md](5-flash.md) | Factory restore and Morse SOS flash commands |
| [factory-firmware-pull.md](factory-firmware-pull.md) | Session log and SHA-256 for completed dumps |
| [FT232R datasheet (FTDI)](https://ftdichip.com/wp-content/uploads/2020/08/DS_FT232R.pdf) | Adapter datasheet |
| [firmware/3202087-lsc-plug/](../../firmware/3202087-lsc-plug/) | Factory `.bin` dumps, `backup.sh` |
| [firmware/3202087-lsc-plug/morse-sos/](../../firmware/3202087-lsc-plug/morse-sos/) | Morse SOS LibreTiny project (demo firmware) |
| [firmware/3202087-lsc-plug/uart-debug/](../../firmware/3202087-lsc-plug/uart-debug/) | Optional UART link verification scripts |
| [docs/images/uart1.png](../images/uart1.png) | UART pads on blue daughterboard |
| [docs/images/uart2.jpg](../images/uart2.jpg) | Pad silkscreen close-up |
| [docs/images/usb-uart-adapter.png](../images/usb-uart-adapter.png) | FT232RL adapter pinout |
| [docs/images/lsc-smart-plug.png](../images/lsc-smart-plug.png) | Product photo |
| [README.md](../../README.md) | Demo overview |

## Session notes (UART debug, 2026-08-28)

Operator session on macOS with FT232RL (`/dev/cu.usbserial-AG0KXO8J`):

- Adapter loopback TXO↔RXI: **pass**
- Plug powered from adapter 3.3 V; LEDs active
- No readable ASCII boot log at 115200 / 74880 / sweep, **consistent with Beken + TX1 used for Wi‑Fi LED**
- UART prep scripts: [firmware/3202087-lsc-plug/uart-debug/](../../firmware/3202087-lsc-plug/uart-debug/) (optional; not required for `backup.sh` pull)

## Avoid / verify

- **Cloudcutter**: reported **not** working for this plug generation (Keet Support, ESPHome Devices).
- **esptool**: wrong tool family; chip is Beken, not Espressif.
- **FCC / article variants**: Action revises silicon inside the same article number; confirm module marking on your unit before locking flash commands.
- **CEN location**: on **Side B** (back edge) of the blue module, **not** on the green mains PCB.
