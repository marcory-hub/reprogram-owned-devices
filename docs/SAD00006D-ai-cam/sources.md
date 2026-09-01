# Elecrow AI Camera SAD00006D: sources

Curated references for the Elecrow AI Camera (model **SAD00006D**).

## Vendor

| Source | URL | What it shows |
| --- | --- | --- |
| Elecrow wiki | https://www.elecrow.com/wiki/AI-Camera-Development-Board-Vision-Sensor-Board-Powered-By-ESP32.html | Product overview, setup, stock behavior |
| Elecrow GitHub | https://github.com/Elecrow-RD/AI_Camera_Development_Board_Vision_Sensor_Board_Powered_By_ESP32 | `factory_firmware/`, examples, schematics |
| GitHub `factory_firmware/` | https://github.com/Elecrow-RD/AI_Camera_Development_Board_Vision_Sensor_Board_Powered_By_ESP32/tree/master/factory_firmware | Published stock `.bin` files (same family as in-repo copy) |

## In-repo assets (this project)

| Path | What it shows |
| --- | --- |
| [mac-host-setup.md](mac-host-setup.md) | **macOS one-time install + both flash commands** |
| [0-feasibility.md](0-feasibility.md) | Chip, USB path, verdict |
| [4-write-firmware.md](4-write-firmware.md) | Custom firmware build path and project layout |
| [2-dump-firmware.md](2-dump-firmware.md) | Stock restore and hardware read |
| [5-flash.md](5-flash.md) | Custom ESP-IDF build and flash (verified 2026-08-31) |
| [ai_camera_module_quick_start_guide.pdf](ai_camera_module_quick_start_guide.pdf) | Vendor quick-start PDF |
| [firmware/SAD00006D-ai-cam/factory-firmware/](../../firmware/SAD00006D-ai-cam/factory-firmware/) | Local copy of published stock bins + Flash Download Tool |
| [firmware/SAD00006D-ai-cam/restore-factory.sh](../../firmware/SAD00006D-ai-cam/restore-factory.sh) | One-command stock restore via `esptool` |
| [requirements/esptool.txt](../../requirements/esptool.txt) | Repo venv dependency for factory restore |
| [firmware/SAD00006D-ai-cam/yolo11n-detect/](../../firmware/SAD00006D-ai-cam/yolo11n-detect/) | Custom offline detection firmware (ESP-IDF) |
| [README.md](../../README.md) | Demo overview and outcome photos |

## Restore stock

Published partition images live under `firmware/SAD00006D-ai-cam/factory-firmware/AICamera_ESP32S3_PCBA-V1.1_260629/`. Flash addresses: [factory-firmware/readme.md](../../firmware/SAD00006D-ai-cam/factory-firmware/readme.md). Source: [Elecrow GitHub `factory_firmware/`](https://github.com/Elecrow-RD/AI_Camera_Development_Board_Vision_Sensor_Board_Powered_By_ESP32/tree/master/factory_firmware).

Procedure: [mac-host-setup.md](mac-host-setup.md#action-a-restore-factory-firmware). Bins and addresses: [2-dump-firmware.md](2-dump-firmware.md#restore-factory-firmware).

Stock was restored on hardware at least once using these bins before custom firmware was flashed.
