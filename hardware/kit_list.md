# Hardware Kit List

Participants should purchase hardware before the workshop. The total cost is approximately $15–25 USD.

---

## Required

### ESP32-S3-CAM Development Board
- **Product**: ESP32-S3-CAM Development Board with OV3660 Camera
- **Module**: ESP32-S3-WROOM N16R8
- **Specs**: 16MB Flash, 8MB PSRAM, OV3660 camera sensor, dual USB-C, WiFi + Bluetooth
- **Amazon**: https://www.amazon.com/dp/B0GDFCCP2G
- **Why this board**: The 8MB PSRAM is the minimum needed to buffer camera frames and run a TFLite model simultaneously. The dual USB-C means one port for power, one for serial flashing — no USB adapter needed.

### USB-C Cable (data-capable)
- Any USB-C cable that supports data transfer (not charge-only)
- Most participants already own one

---

## Optional (event output demo)

- Breadboard (half-size or full)
- 3× LEDs (any color)
- 3× 220Ω resistors
- Jumper wires (male-to-male)

These are used in the optional Part 2 extension: lighting an LED when a specific class is detected.

---

## Instructor Hardware

The workshop is developed and tested on:
- ESP32-S3-WROOM N16R8 (B0GDFCCP2G) — primary device
- macOS host, verified also on Ubuntu 22.04 and Windows 11

---

## Not Required

- External USB-to-serial adapter (the board has one built in)
- External FTDI programmer
- Any ESP-IDF hardware flasher dongle
- Cloud GPU or paid ML service account
