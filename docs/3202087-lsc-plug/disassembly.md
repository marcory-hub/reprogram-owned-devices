# Factory firmware disassembly: LSC 3202087.2 (BK7238)

Static analysis of the stock flash dump. The `.bin` is machine code and Tuya data, not source you can edit. Use this doc to understand what the image contains; use [2-dump-firmware.md](2-dump-firmware.md) to pull or restore it.

**Dump analyzed:** `firmware/3202087-lsc-plug/factory-20260828-2101-bk7238.bin` (2097152 bytes, SHA-256 in [factory-firmware-pull.md](factory-firmware-pull.md)).

## What you can and cannot get

| You can | You cannot |
| --- | --- |
| Identify SDK, cloud endpoints, drivers, data-point names | Recover C source or rebuild the firmware |
| Confirm BL0937, relay, BLE, and OTA architecture | Read UUID/auth keys from encrypted flash regions |
| Disassemble Thumb code after CRC strip | Get clean Ghidra pseudocode without manual RE work |
| Keep the dump for rollback | Patch behavior by editing the `.bin` |

To change behavior, flash **replacement** firmware (OpenBeken, ESPHome, LibreTiny) using the GPIO map in [0-feasibility.md](0-feasibility.md).

## Prerequisites

- Factory dump verified (2097152 bytes). See [2-dump-firmware.md](2-dump-firmware.md).
- `ltchiptool` in repo `.venv` (`requirements/ltchiptool.txt`).
- Optional: Ghidra (installed on operator Mac via Homebrew), `arm-none-eabi-objdump` from Arm GNU Toolchain.

## 1. Split the raw dump (required)

Bootloader and app partitions embed **CRC16 every 32 bytes**. Disassembling the raw 2 MiB file directly interleaves garbage with code.

From repo root:

```bash
.venv/bin/ltchiptool flash split t1-2s \
  firmware/3202087-lsc-plug/factory-20260828-2101-bk7238.bin \
  -o /tmp/bk7238-split
```

Use board `t1-2s` (T1-2S-NL module on this plug). `generic-bk7238-tuya` is equivalent for partition layout.

Expected outputs (sizes vary slightly by dump):

| File | Flash offset | Role |
| --- | --- | --- |
| `000000_bootloader_*.bin` | `0x000000` | Bootloader (~68 KiB) |
| `011000_app_*.bin` | `0x011000` | Tuya application (~1.0 MiB) |
| `12C000_download_*.bin` | `0x12C000` | OTA slot (empty on this dump) |
| `1E3000_calibration_*.bin` | `0x1E3000` | RF/flash calibration TLVs |
| `1E4000_kvs_*.bin` | `0x1E4000` | Key-value store |
| `1EC000_userdata_*.bin` | `0x1EC000` | User data (encrypted/binary) |
| `1F5000_tuya_*.bin` | `0x1F5000` | Tuya storage (encrypted/binary) |

Partition layout matches LibreTiny [Generic BK7238 (Tuya T1)](https://docs.libretiny.eu/boards/generic-bk7238-tuya/).

`ltchiptool flash file` on the raw dump reports: **Raw ARM Binary**.

## 2. Flash region map (full dump)

64 KiB block summary for the backed-up image:

| Range | Content |
| --- | --- |
| `0x000000`-`0x10ffff` | Bootloader + app code and rodata |
| `0x110000`-`0x11ffff` | Mostly empty (`0xFF`) |
| `0x120000`-`0x1dffff` | Empty (no pending OTA image) |
| `0x1e0000`-`0x1effff` | Calibration, KVS, userdata tail |
| `0x1f0000`-`0x1fffff` | Tuya storage and tail data |

## 3. Stock firmware identity (strings)

| Item | Value |
| --- | --- |
| SoC | Beken **BK7238** (marker at flash `0x110` and app header) |
| Platform | Tuya **T1** |
| App version | `T1_2.0.0` |
| SDK bundle | `beta.18_T1_wifi_ble_com_0.0.4` |
| IoT SDK | `tuyaos-iot_3.8.31` |
| Build stamp | `2025_01_02_15_55_27` (CI) |
| Compile date string | `Apr 11 2025` |

## 4. Architecture and load addresses

| Item | Detail |
| --- | --- |
| CPU | ARMv5TE (Beken BK7238) |
| Bootloader | ARM mode; vector table at flash `0x0` |
| Application | **Thumb** for main code |
| App load base | **`0x08011000`** (flash offset `0x011000` in CPU map) |
| Known-good code region | **`0x08060000`** and surrounding blocks (`0x08020000`-`0x08100000`) |

Vector table at app start points to Thumb handlers in the `0x08200000`-`0x08800000` range (Beken memory map).

## 5. Cloud, TLS, and OTA (strings)

| Category | Indicators |
| --- | --- |
| DNS / cloud | `h3.iot-dns.com`, `h3-%s.iot-dns.com` |
| HTTPS | `https://%s/v1/dns_query`, `/v1/url_config`, `/v1/root_CAca`, `/v2` |
| MQTT | `MQTT AND V=2.3`, `TUYA_TLS`, **mbedtls** |
| OTA | `diff2ya` delta patch header and CRC checks |
| Provisioning | BLE stack (`ble_hs_*`, `tuya_ble_*`), Wi-Fi WPA (`hostap_beken`, EAPOL) |

Typical stock risks: cloud-only control, OTA trust chain, provisioning attack surface. **Cloudcutter** does not apply to this generation ([sources.md](sources.md)).

## 6. Hardware drivers (matches GPIO map)

Strings and module names align with [0-feasibility.md](0-feasibility.md) ESPHome Devices GPIO map.

**Energy (BL0937):**

```text
bl0937 sel pin: %d, level: %d
bl0937 cf1 pin: %d
bl0937 cf pin: %d
```

Sources: `tdd_energy_monitor_bl0937_hlw8012.c`, `elec_energy_monitor.c`.

**Relay / button / LEDs:**

```text
qrelay_1, led_1, led_2, ...
app_elec_button.c, app_elec_led.c, tdd_relay_elec.c
```

Maps to **P24** relay, **P26** button, **P9** / **P11** LEDs.

**Other features visible in strings:** countdown timers, child lock, overcharge protection, power calibration, energy upload over MQTT when cloud connected.

## 7. Disassembly commands

### Strings scan (low effort)

```bash
strings -n 8 /tmp/bk7238-split/011000_app_*.bin | rg -i 'tuya|mqtt|bl0937|relay|h3\.|diff2ya|productKey'
```

### objdump (Thumb, stripped app)

Extract a slice at file offset `0x4F000` (linked address `0x08060000`):

```bash
APP=/tmp/bk7238-split/011000_app_*.bin
dd if=$APP bs=1 skip=$((0x4F000)) count=4096 of=/tmp/code4k.bin
arm-none-eabi-objdump -D -b binary -m arm -M force-thumb \
  --adjust-vma=0x08060000 /tmp/code4k.bin | less
```

Output should show plausible Thumb (`push`, `ldr`, `bl`), not data interpreted as code.

### Ghidra (interactive)

1. Import `011000_app_*.bin` from the split output.
2. Language: **ARM v5 32 little-endian**.
3. Base address: **`0x08011000`**.
4. Run auto-analysis, then disassemble from **`0x08060000`**.

Headless run on the stripped app finds on the order of **160+ functions**, but **string cross-refs often do not link** without manual entry points and segment setup. Expect assembly and partial pseudocode, not readable `main()`-level C.

Do **not** import the raw 2 MiB dump without splitting; CRC bytes break disassembly.

## 8. Userdata and secrets

Partitions `1EC000_userdata_*.bin` and `1F5000_tuya_*.bin` are **encrypted or binary**. A strings pass shows no plain-text UUID, auth key, or Wi-Fi credentials. Debug format strings in the app reference SSID/password (common in Tuya SDK builds); that is not proof a debug build ships on device.

## 9. Practical outcomes

| Goal | Approach |
| --- | --- |
| Understand stock behavior | Strings + split + this doc |
| Security audit of stock | Black-box tests on hardware, or deep Ghidra RE (high effort) |
| Change relay/energy/cloud behavior | Flash replacement firmware; keep factory `.bin` for restore |
| Roll back to stock | [5-flash.md](5-flash.md) restore path |

## References

- [0-feasibility.md](0-feasibility.md), hardware, GPIO, verdict
- [2-dump-firmware.md](2-dump-firmware.md), pull and verify
- [factory-firmware-pull.md](factory-firmware-pull.md), session log and SHA-256
- [sources.md](sources.md), external guides
- [LibreTiny BK7238 Tuya partitions](https://docs.libretiny.eu/boards/generic-bk7238-tuya/)
- [ltchiptool](https://docs.libretiny.eu/docs/flashing/tools/ltchiptool/)
