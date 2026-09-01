# Elecrow AI Camera SAD00006D: decide changes

**Device folder:** `SAD00006D-ai-cam`

## Step 1: Stock firmware summary

| Area | Stock behavior |
| --- | --- |
| Cloud | Streams audio and video to [xiaozhi.me](https://xiaozhi.me/) |
| Recognition | Online via vendor cloud; not documented for offline use |
| Setup | Wi‑Fi hotspot pairing (e.g. `AI_Camera_B3E4`) |
| Local UI | LCD prompts for network join |

Photo: [factory-firmware-login.png](../images/factory-firmware-login.png).

## Step 2: Local-first improvement directions

1. **Offline object detection**: run a model on the ESP32-S3, draw boxes on the LCD, no Wi-Fi.
2. **On-device model menu**: pick detection model at boot from the LCD.
3. **Confidence threshold**: tune false positives without recompiling.
4. **Display power**: wake/sleep LCD; auto-off after idle.
5. **microSD snapshots**: save detection frames locally.

## Step 3: Critical ranking

| Direction | Payoff | Risk | Reversibility |
| --- | --- | --- | --- |
| Offline detection | High | Medium (flash size, perf) | High if factory dump kept |
| Model menu + threshold | High | Low | High |
| microSD capture | Medium | Low | High |
| Drop cloud entirely | High | Medium | High |

**Implemented in this repo:** directions 1-5 in `firmware/SAD00006D-ai-cam/yolo11n-detect/`.

## References

- [4-write-firmware.md](4-write-firmware.md)
- [sources.md](sources.md)
- [README.md](../../README.md), outcome photos
