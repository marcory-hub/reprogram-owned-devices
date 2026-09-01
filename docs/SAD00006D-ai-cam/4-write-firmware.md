# Elecrow AI Camera SAD00006D: write firmware

**Device folder:** `SAD00006D-ai-cam`

Prerequisite: [3-decide-changes.md](3-decide-changes.md). Factory backup: [2-dump-firmware.md](2-dump-firmware.md). **macOS host install:** [mac-host-setup.md](mac-host-setup.md).

## Goal

Run YOLO11n object detection fully on-device with LCD model selection, confidence tuning, and optional microSD snapshots. No cloud account.

## Project path

```
firmware/SAD00006D-ai-cam/yolo11n-detect/
```

ESP-IDF project, **esp32s3**, 16 MiB flash. Espressif `esp-dl`, `esp32-camera`, custom UI in `main/`.

## Files to touch (typical changes)

| Area | Path |
| --- | --- |
| Detection logic | `main/` |
| Models | `components/vespa_detect/models/` |
| UI / menus | `main/ui/` |
| Board config | `main/board/`, `sdkconfig` |
| Partitions | `partitions.csv` |

## Build

Host setup (ESP-IDF, cmake, CH340): [mac-host-setup.md](mac-host-setup.md).

```bash
source ~/esp/esp-idf/export.sh
cd firmware/SAD00006D-ai-cam/yolo11n-detect
idf.py set-target esp32s3   # first checkout only
idf.py build
```

Flash: [5-flash.md](5-flash.md). Do not use `restore-factory.sh` for custom images.

## Success criteria

- [ ] Boots to model selection menu on LCD
- [ ] Detects objects offline with bounding boxes
- [ ] Confidence threshold adjustable on device
- [ ] Factory restore path documented

## Rollback

[2-dump-firmware.md](2-dump-firmware.md#restore-factory-firmware) or [mac-host-setup.md](mac-host-setup.md#action-a-restore-factory-firmware).

## References

- [mac-host-setup.md](mac-host-setup.md)
- [5-flash.md](5-flash.md)
- [0-feasibility.md](0-feasibility.md)
