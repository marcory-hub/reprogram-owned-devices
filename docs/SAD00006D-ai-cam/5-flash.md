# Elecrow AI Camera SAD00006D: flash

**Device folder:** `SAD00006D-ai-cam`

**macOS host install:** [mac-host-setup.md](mac-host-setup.md). Custom vs stock layouts differ; use this page for ESP-IDF only.

## Flash custom firmware

Prerequisite: [mac-host-setup.md](mac-host-setup.md) steps 1, 2, 4 and `idf.py build` ([4-write-firmware.md](4-write-firmware.md)).

```bash
source ~/esp/esp-idf/export.sh
cd firmware/SAD00006D-ai-cam/yolo11n-detect
idf.py -p /dev/cu.wchusbserial1120 flash
```

Optional monitor: `idf.py -p PORT monitor` (exit **Ctrl+]**).

Verified **2026-08-31**: ESP32-S3 over CH340, hash verified, model picker on LCD.

## Post-flash (custom)

- Model selection (4 / 2 / 1 class), not Wi‑Fi setup
- Offline bounding boxes
- Right button: display wake/sleep

Photos: [images/IMG_4108.png](images/IMG_4108.png), [images/IMG_4111.png](images/IMG_4111.png), [images/IMG_4130.png](images/IMG_4130.png).

Troubleshooting and rollback: [mac-host-setup.md](mac-host-setup.md).

## References

- [mac-host-setup.md](mac-host-setup.md)
- [2-dump-firmware.md](2-dump-firmware.md)
- [4-write-firmware.md](4-write-firmware.md)
