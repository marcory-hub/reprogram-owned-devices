# Elecrow AI Camera SAD00006D: dump firmware

**Device folder:** `SAD00006D-ai-cam`

Read-only factory backup before any custom write. Restore path documented below.

Prerequisite: [1-connect.md](1-connect.md) (USB port confirmed).

## What the `.bin` is (and is not)

A factory image is **machine code + data**, not source you can edit in Cursor. Keep it for rollback.

| You can | You cannot |
| --- | --- |
| Keep it as a **rollback** copy of stock | Read it like normal firmware source |
| Restore with vendor bins or a full flash dump | Patch behavior by editing the `.bin` in place |

## Published stock (in repo)

Elecrow publishes factory images on [GitHub `factory_firmware/`](https://github.com/Elecrow-RD/AI_Camera_Development_Board_Vision_Sensor_Board_Powered_By_ESP32/tree/master/factory_firmware). This repo keeps a local copy:

```
firmware/SAD00006D-ai-cam/factory-firmware/AICamera_ESP32S3_PCBA-V1.1_260629/
├── AI_Camera-V1.0-Arduino.ino.bootloader.bin   → 0x0
├── AI_Camera-V1.0-Arduino.ino.partitions.bin   → 0x8000
├── boot_app0.bin                               → 0xe000
└── AI_Camera-V1.0-Arduino.ino.bin              → 0x10000
```

Addresses from [factory-firmware/readme.md](../../firmware/SAD00006D-ai-cam/factory-firmware/readme.md). Stock was **restored on hardware at least once** using these files before custom firmware was flashed.

If this folder is missing, re-download from the [Elecrow GitHub repo](https://github.com/Elecrow-RD/AI_Camera_Development_Board_Vision_Sensor_Board_Powered_By_ESP32/tree/master/factory_firmware) into `firmware/SAD00006D-ai-cam/factory-firmware/`.

## Restore factory firmware

Prerequisite: [mac-host-setup.md](mac-host-setup.md) (CH340 driver + repo `.venv`).

### Flash stock bins

**Recommended:** run the restore script (auto-picks the port when only one match exists):

```bash
firmware/SAD00006D-ai-cam/restore-factory.sh /dev/cu.wchusbserial1120
```

Replace the port with yours ([mac-host-setup.md](mac-host-setup.md#port-check)).

**Manual `esptool` (same images and addresses):**

```bash
FACTORY=firmware/SAD00006D-ai-cam/factory-firmware/AICamera_ESP32S3_PCBA-V1.1_260629
.venv/bin/esptool.py --chip esp32s3 -p PORT -b 460800 \
  write_flash --flash_mode dio --flash_size 16MB \
  0x0     "$FACTORY/AI_Camera-V1.0-Arduino.ino.bootloader.bin" \
  0x8000  "$FACTORY/AI_Camera-V1.0-Arduino.ino.partitions.bin" \
  0xe000  "$FACTORY/boot_app0.bin" \
  0x10000 "$FACTORY/AI_Camera-V1.0-Arduino.ino.bin"
```

**Vendor GUI alternative:** Espressif Flash Download Tool; see [factory-firmware/readme.md](../../firmware/SAD00006D-ai-cam/factory-firmware/readme.md) and screenshots in that folder.

### After restore

- Press **RESET** if the LCD does not update.
- Stock firmware shows the Wi‑Fi setup flow (hotspot name like `AI_Camera_*`), not the custom offline detection menu.
- See [README.md](../../README.md) factory-firmware screenshot for expected stock UI.
- To return to custom firmware later: [5-flash.md](5-flash.md) (different partition layout; do not reuse this restore command).

Stock was **restored on hardware at least once** using these bins before custom firmware was flashed.

## Full flash read from hardware (optional)

For a single rollback file matching the chip contents, read the full **16 MiB** flash:

Flash size: **16 MiB** (16777216 bytes) per [0-feasibility.md](0-feasibility.md).

```bash
esptool.py -p PORT -b 460800 read_flash 0 0x1000000 \
  firmware/SAD00006D-ai-cam/factory-$(date +%Y%m%d-%H%M)-esp32s3.bin
```

Replace `PORT` with your `/dev/cu.usbmodem*`, `/dev/cu.usbserial*`, or `/dev/cu.wchusbserial*` device.

## Verify

```bash
wc -c firmware/SAD00006D-ai-cam/factory-firmware/AICamera_ESP32S3_PCBA-V1.1_260629/*.bin
shasum -a 256 firmware/SAD00006D-ai-cam/factory-firmware/AICamera_ESP32S3_PCBA-V1.1_260629/*.bin
```

Full-chip dump check:

```bash
wc -c firmware/SAD00006D-ai-cam/factory-*-esp32s3.bin
```

| Check | Expected |
| --- | --- |
| Full read size | **16777216** bytes (16 MiB) |
| Partial file | Delete and re-run if smaller |

## Safety checklist

- [ ] Read-only command only for `read_flash` (no `write_flash` unless restoring stock)
- [ ] Correct USB port
- [ ] Published stock bins present before custom flash
- [ ] For restore on macOS: CH340 driver installed and `/dev/cu.wchusbserial*` or similar visible

## References

- [mac-host-setup.md](mac-host-setup.md)
- [sources.md](sources.md)
- [0-feasibility.md](0-feasibility.md)
- [5-flash.md](5-flash.md)
- [firmware/SAD00006D-ai-cam/factory-firmware/readme.md](../../firmware/SAD00006D-ai-cam/factory-firmware/readme.md)
