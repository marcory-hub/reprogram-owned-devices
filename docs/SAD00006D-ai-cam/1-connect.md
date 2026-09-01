# Elecrow AI Camera SAD00006D: connect

**Device folder:** `SAD00006D-ai-cam`

Prerequisite: [0-feasibility.md](0-feasibility.md).

## USB-C path

1. Connect the board to the host with a **USB-C** cable.
2. Power indicator should light (5 V from USB).
3. No soldering, no USB-TTL adapter.

## macOS serial port

Port check and CH340 driver: [mac-host-setup.md](mac-host-setup.md).

```bash
ls /dev/cu.usbmodem* /dev/cu.usbserial* /dev/cu.wchusbserial* 2>/dev/null
```

After the WCH driver, expect `/dev/cu.wchusbserial*` (e.g. `1120`).

## Boot / flash mode

ESP32-S3 enters serial bootloader over USB when the tool requests it. If `idf.py flash` cannot connect, hold **BOOT**, tap **RESET**, release **BOOT**, then retry [to be verified on your unit].

## Link check

After plug-in, confirm the port exists. Optional monitor:

```bash
idf.py -p PORT monitor
```

Stock firmware may show Wi‑Fi setup prompts rather than a ROM banner.

## Safety

- USB **5 V** only. No mains wiring on this board.
- Use a data-capable USB-C cable (charge-only cables fail silently).

## References

- [0-feasibility.md](0-feasibility.md)
- [mac-host-setup.md](mac-host-setup.md)
- [2-dump-firmware.md](2-dump-firmware.md)
