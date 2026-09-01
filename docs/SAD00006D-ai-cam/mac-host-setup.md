# SAD00006D: macOS host setup

**Start here** on a Mac before factory restore or custom flash. Verified **2026-08-31** on this project's MacBook (Apple Silicon, macOS 26.x).

Device procedures: stock [2-dump-firmware.md](2-dump-firmware.md#restore-factory-firmware), custom [5-flash.md](5-flash.md).

## What gets installed (disk paths)

| Component | Path / manifest | Used for |
| --- | --- | --- |
| Homebrew | `/opt/homebrew` | cmake, ninja |
| cmake, ninja | `/opt/homebrew/bin/cmake`, `ninja` | ESP-IDF build |
| WCH CH340 driver | `/Applications/CH34xVCPDriver.app` + system extension `cn.wch.CH34xVCPDriver` | USB serial port |
| Repo Python venv | `<repo>/.venv/` | Factory restore (`esptool`) |
| esptool (repo) | `<repo>/.venv/bin/esptool.py` | `restore-factory.sh` |
| esptool dep file | `<repo>/requirements/esptool.txt` | venv install |
| ESP-IDF **v5.5.1** | `~/esp/esp-idf/` | Custom build/flash |
| ESP-IDF tools | `~/.espressif/` (Python env, xtensa gcc, esptool) | Custom build/flash only |
| Restore script | `<repo>/firmware/SAD00006D-ai-cam/restore-factory.sh` | Stock flash |
| Stock bins | `<repo>/firmware/SAD00006D-ai-cam/factory-firmware/AICamera_ESP32S3_PCBA-V1.1_260629/` | Stock flash |
| Custom project | `<repo>/firmware/SAD00006D-ai-cam/yolo11n-detect/` | Custom build/flash |

**Not in repo:** ESP-IDF, `.venv/`, `~/.espressif/`, CH340 app, Homebrew packages.

## One-time install (run once per Mac)

### 1. Homebrew + build tools

```bash
brew install cmake ninja
```

Homebrew itself: [brew.sh](https://brew.sh) if missing.

### 2. CH340 USB serial driver

Board enumerates as **USB Serial** (vendor `0x1A86`, product `0x7522`). Safe Mode **not** required.

1. Download [CH34XSER_MAC](https://www.wch.cn/downloads/CH34XSER_MAC_ZIP.html) or `brew install --cask wch-ch34x-usb-serial-driver` (needs admin password in Terminal).
2. Open `CH34xVCPDriver.dmg`, drag **CH34xVCPDriver** to **Applications** (required).
3. Run `/Applications/CH34xVCPDriver.app`, click **Install**.
4. **System Settings → General → Login Items & Extensions → Driver Extensions** → enable **CH34xVCPDriver**.
5. Reboot, unplug/replug camera.

Check:

```bash
systemextensionsctl list | grep -i ch34
ls /dev/cu.wchusbserial* 2>/dev/null
```

Expect `[activated enabled]` and a port like `/dev/cu.wchusbserial1120` (suffix varies). `zsh: no matches found` means no port yet.

### 3. Repo venv (factory restore)

From repo root:

```bash
python3 -m venv .venv
.venv/bin/pip install -r requirements/esptool.txt
.venv/bin/esptool.py version
```

### 4. ESP-IDF v5.5.1 (custom firmware)

Matches project `CONFIG_IDF_INIT_VERSION="5.5.1"`.

```bash
mkdir -p ~/esp
git clone -b v5.5.1 --recursive --depth 1 https://github.com/espressif/esp-idf.git ~/esp/esp-idf
cd ~/esp/esp-idf
./install.sh esp32s3
```

Pulls toolchain into `~/.espressif/` (separate from repo `.venv`).

## Every new shell

**Custom build/flash:**

```bash
source ~/esp/esp-idf/export.sh
```

**Factory restore:** no export needed; use `<repo>/.venv/bin/esptool.py` or the restore script.

## Port check

```bash
ls /dev/cu.usbmodem* /dev/cu.usbserial* /dev/cu.wchusbserial* 2>/dev/null
```

Use the `wchusbserial` device on this Mac after CH340 install.

## Action A: restore factory firmware

Prerequisite: steps 2 and 3 above. **Do not** use ESP-IDF or `idf.py` (Arduino partition layout).

```bash
cd <repo>
firmware/SAD00006D-ai-cam/restore-factory.sh /dev/cu.wchusbserial1120
```

Press **RESET** if LCD stays on Wi‑Fi setup screen. Full detail: [2-dump-firmware.md](2-dump-firmware.md#restore-factory-firmware).

## Action B: build + flash custom firmware

Prerequisite: steps 1, 2, and 4 above.

```bash
source ~/esp/esp-idf/export.sh
cd <repo>/firmware/SAD00006D-ai-cam/yolo11n-detect
idf.py set-target esp32s3    # first checkout only
idf.py build
idf.py -p /dev/cu.wchusbserial1120 flash
```

First `idf.py build` takes several minutes (managed components + ML model pack). Expect model picker on LCD, not Wi‑Fi setup. Full detail: [4-write-firmware.md](4-write-firmware.md), [5-flash.md](5-flash.md).

Optional log: `idf.py -p PORT monitor` (exit **Ctrl+]**).

## Do not mix

| | Factory stock | Custom ESP-IDF |
| --- | --- | --- |
| Tool | `.venv/bin/esptool.py` / `restore-factory.sh` | `idf.py flash` |
| Partition offset | `0x8000` (Arduino) | `0x9000` (project `partitions.csv`) |
| App address | `0x10000` vendor `.bin` | `0x10000` `yolo11_detect.bin` |

## Troubleshooting

| Symptom | Fix |
| --- | --- |
| App must be in `/Applications` | Copy CH34xVCPDriver from DMG to `/Applications`, run from there |
| No `/dev/cu.*` | Finish driver steps, reboot, replug |
| `cmake` must be on PATH | `brew install cmake ninja` |
| `idf.py` not found | `source ~/esp/esp-idf/export.sh` |
| Flash cannot connect | Data USB-C cable; hold **BOOT**, tap **RESET**, release **BOOT** |
| Blank LCD | Press **RESET** |

## References

- [1-connect.md](1-connect.md)
- [2-dump-firmware.md](2-dump-firmware.md)
- [4-write-firmware.md](4-write-firmware.md)
- [5-flash.md](5-flash.md)
