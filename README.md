# Who controls your smart devices?

- You are fully in control...
![QR code linking to the GitHub repository](docs/images/github-repo-qr.png)
- You can
  - choose what your device does
  - use hardware after official support ends
  - have fun

![](docs/images/emoji/video-game.svg)

## Why reprogram with Cursor?

It speeds up
- research
- brainstorm
- coding, building, and flashing your firmware

![](docs/images/emoji/hammer-and-wrench.svg)

---



## You can treat firmware like any other codebase

- it directly controls hardware
- .bin, so not human-readable
- but you can rewrite the firmware in C and C++
- compile it and flash it

![](docs/images/emoji/brain.svg)

---



## Quick start

- 0 Confirm that your device can be reprogrammed
- 1 Connect the smart device to your computer
- 2 Pull the factory firmware
- 3 Make a plan
- 4 Write code
- 5 Flash your own firmware

![](docs/images/emoji/rocket.svg)

---



### 0. Check reprogram feasibility

- Before bricking the device ask for:
  - uncertainties
  - verify `@critical` the `docs/<device>/0-feasibility.md`

[Example prompt to check feasibility](docs/cursor-example-prompts/0-feasibility.md)

---



### 1. Connect the board from the smart device to your computer

- OTA
- USB-C
- USB adapter

[Example prompt to connect the device](docs/cursor-example-prompts/1-connect.md)

![](docs/images/emoji/wifi.svg)

---



### 2. Pull the factory firmware

[Example prompt to pull the factory firmware](docs/cursor-example-prompts/2-dump-firmware.md)

![](docs/images/emoji/floppy-disk.svg)

---



### 3. Decide what your device should do

[Example prompt to brainstorm](docs/cursor-example-prompts/3-decide-changes.md)

![](docs/images/emoji/shrug-man.svg)

---



### 4. Write custom firmware in Cursor

[Example prompt to write custom firmware](docs/cursor-example-prompts/4-write-firmware.md)

![](docs/images/emoji/laptop.svg)

---



### 5. Flash your firmware

[Example prompt to flash](docs/cursor-example-prompts/5-flash.md)

![](docs/images/emoji/camera-with-flash.svg)

---



## Devices ready for hands-on reprogramming

- Elecrow AI camera
- Smart plug

![](docs/images/emoji/ghost.svg)


### [Elecrow AI Camera (SAD00006D)](https://www.elecrow.com/wiki/AI-Camera-Development-Board-Vision-Sensor-Board-Powered-By-ESP32.html)

![Elecrow AI Camera with custom firmware running on-device object detection](docs/images/SAD00006D-ai-cam.png)

- Quick start guide is entirely cloud-oriented [xiaozhi.me](https://xiaozhi.me/).
![Elecrow AI Camera stock firmware asking to join Wi-Fi hotspot AI_Camera_B3E4](docs/images/factory-firmware-login.png)



#### Once reprogrammed

- Reboot (left button)
- Wake (right button)
- Capture detections on a microSD card
![Elecrow AI Camera model selection menu on the LCD](docs/SAD00006D-ai-cam/images/IMG_4108.png)

![Elecrow AI Camera confidence threshold selection on the LCD](docs/SAD00006D-ai-cam/images/IMG_4111.png)

![Elecrow AI Camera detecting a hornet offline with bounding box and confidence score](docs/SAD00006D-ai-cam/images/IMG_4130.png)

---



### [LSC Smart Connect slimme stekker](https://www.action.com/nl-nl/p/3202087/lsc-smart-connect-slimme-stekker/)

![LSC Smart Connect smart plug](docs/images/lsc-smart-plug.png)

#### Open the plug and wire UART

![Opening the LSC smart plug with plastic cards in the corner seams](docs/images/gamma.png)

![Disassembled plug showing the blue UART module and shell pieces](docs/images/uart1.png)

![UART pad silkscreen on the blue module (3V3, GND, RX1, TX1, P9, P24)](docs/images/uart2.png)

![Soldered UART wires on the blue module (red 3V3, black GND, white RX1, yellow TX1)](docs/images/soldering.png)

![FT232RL USB-TTL adapter pin labels (DTR, RX, TX, VCC, CTS, GND)](docs/images/usb-uart-adapter.png)

![LSC Smart Connect plug relay LED blinking Morse SOS after custom LibreTiny firmware flash](docs/images/lsc-plug-morse-sos-demo.gif)

#### More usefull custom firmware with some smarter ideas

## ![](docs/images/emoji/light-bulb.svg)



## Flash warning

- Opening the device voids warranty.
- You can brick the device if wiring, dump, or flash goes wrong. Back up factory firmware first.
- Only modify hardware you **own** or have explicit permission to repair or open.
- EU Radio Equipment Directive (RED) is pushing more locked firmware; check feasibility before you open anything.
![Warning](docs/images/emoji/warning.svg)

---



## Key safety reminders

- Use **caution** when working with mains voltage (230V).
![Lightning warning](docs/images/emoji/lightning.svg)

---



## Learn more

- [Banned-book library inside a Wi-Fi smart bulb](https://gikiewicz.com/banned-book-library-in-a-wi-fi-smart-light-bulb)
- [Everything I Own, Owned](https://schlarp.com/posts/everything-i-own-owned/)

