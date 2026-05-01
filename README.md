# Embedded Vision AI on ESP32

Workshop materials for **Embedded Vision AI on ESP32: Building Real-Time Edge Intelligence with Microcontrollers** (Packt Live Online Workshop, 5 hours).

## Repository Layout

```
proposal/          Workshop overview and agenda (Packt submission)
hardware/          Hardware kit list with purchase links
labs/              Step-by-step lab instructions (Markdown)
slides/            Full 60-slide deck with speaker notes (Markdown)
firmware/          ESP32-S3 firmware projects (ESP-IDF v5.4.1)
  ├── camera_test/ Lab 0 — WiFi MJPEG stream
  ├── lab_01/      Lab 1 — camera throughput benchmark
  ├── lab_02/      Lab 2 — TFLM person detection inference
  └── tools/       model_budget.py · fetch_model.sh · build.sh
```

## Quick Links

- [Workshop Overview](proposal/workshop_overview.md)
- [Hardware Kit List](hardware/kit_list.md)
- [Lab 0 — Environment Setup](labs/lab_00_setup.md)
- [Lab 1 — Embedded Vision Fundamentals](labs/lab_01_fundamentals.md)
- [Lab 2 — Deploying an AI Vision Model](labs/lab_02_deploy_model.md)
- [Slide Deck (60 slides + speaker notes)](slides/slides.md)
- [Firmware README](firmware/README.md)

## Target Hardware

**ESP32-S3-WROOM N16R8** with OV3660 camera — 16MB Flash, 8MB PSRAM octal 80MHz, dual-core Xtensa LX7 @ 240MHz, WiFi + BT, dual USB-C.  
Amazon: https://www.amazon.com/dp/B0GDFCCP2G (~$15)

## Verified Results (hardware, 2026-05-01)

| Firmware | What it does | Measured |
|----------|-------------|---------|
| camera_test | MJPEG stream over WiFi | ~25 FPS live at http://192.168.x.x |
| lab_01 | QVGA JPEG throughput | 27.8 FPS |
| lab_02 | Person detection (TFLM) | 2.6 FPS, 382ms inference |
