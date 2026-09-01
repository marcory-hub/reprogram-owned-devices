# LSC Smart Connect 3202087.2: connect

**Device folder:** `3202087-lsc-plug`

Safe debug access before any firmware read or write. Prerequisite: [0-feasibility.md](0-feasibility.md).

## USB-TTL adapter

Use a **3.3 V** USB-TTL adapter (FT232RL or similar). Adapter datasheet: [FT232R (FTDI)](https://ftdichip.com/wp-content/uploads/2020/08/DS_FT232R.pdf).

Product photo and pin labels: [usb-uart-adapter.png](../images/usb-uart-adapter.png).

Cross TX/RX. Leave DTR/CTS empty on FT232-style adapters.

## Open the plug

Photos: [gamma.png](../images/gamma.png), [uart1.png](../images/uart1.png), [uart2.jpg](../images/uart2.jpg), [soldering.png](../images/soldering.png).

## UART pads (blue module)

The blue **T1-2S-NL** module has labelled castellations on **two edges** [ESPHome Devices]:

| Edge | Pads (top → bottom) |
| --- | --- |
| Side A (front, UART power and serial) | **3V3**, **GND**, **RX1**, **TX1**, **P9**, **P24** |
| Side B (back, opposite edge) | **CEN**, **P1**, **P8**, **P6**, **P26** |

![UART side](../images/uart2.jpg) shows Side A.

### Wiring to adapter

| Target pad | Adapter pin |
| --- | --- |
| 3V3 | VCC (jumper on **3.3 V**) |
| GND | GND |
| RX1 | TXO (adapter TX) |
| TX1 | RXI (adapter RX) |

### CEN (flash / download mode)

**CEN** is chip enable (reset). It sits on **Side B**, the **back edge** of the blue module, opposite **3V3/GND/RX1/TX1**.

**CEN is not on the green mains PCB.**

- Solder or tack a **fifth wire** to the **CEN** pad on the blue module.
- Leave the far end **free** (do not solder CEN to GND).
- During backup/flash: briefly touch the free end to **GND** (~0.25 s), then release.

## Electrical safety

- For UART, CEN, and flash: **no 230 V** on the plug. Power from the **3.3 V** USB-TTL adapter only.
- Before applying VCC: measure **3V3 ↔ GND** in **Ω** mode (not continuity beep). Hard short **0-5 Ω** → stop. Stable **kΩ** range → OK.
- Adapter VCC can run the board for link checks; if flash is unstable, use a separate **3.3 V** supply with common GND [ESPHome Devices].

## Serial link check

### macOS port

```bash
ls /dev/cu.usbserial*
```

Example from operator session: `/dev/cu.usbserial-AG0KXO8J` (serial varies per adapter).

### Loopback (optional)

Short adapter **TXO ↔ RXI**, then:

```bash
.venv/bin/python -c "
import serial, time
p='/dev/cu.usbserial-AG0KXO8J'
s=serial.Serial(p,115200,timeout=0.5)
s.write(b'ping')
time.sleep(0.2)
print(s.read(64))
s.close()
"
```

Expect `b'ping'`.

### Expectations on the plug

| Observation | Meaning |
| --- | --- |
| Adapter loopback PASS | Mac + FT232 OK |
| Plug LEDs blink on USB power | Module booting |
| No ASCII boot text at any baud | **Normal** for this family, not ESP ROM at 74880 |
| Short garbage/null bursts | TX1 shared with Wi‑Fi LED (P11); logging often disabled in configs |

Do **not** use missing boot log as proof of bad wiring if power and loopback are good.

Optional UART scripts: [firmware/3202087-lsc-plug/uart-debug/](../../firmware/3202087-lsc-plug/uart-debug/).

## References

- [0-feasibility.md](0-feasibility.md)
- [sources.md](sources.md)
- [ESPHome Devices, LSC 3202087.2](https://devices.esphome.io/devices/lsc-plug-32020872/)
