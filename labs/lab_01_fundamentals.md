# Lab 1 — Embedded Vision Fundamentals

**Duration:** 90 minutes  
**Goal:** Understand the edge AI landscape and constraints, then run a live inference demo on the ESP32.

---

## Concepts Covered

- Edge AI vs cloud AI — latency, privacy, cost, connectivity tradeoffs
- ESP32-S3 hardware architecture — CPU cores, PSRAM, camera interface
- The vision inference pipeline: capture → preprocess → infer → act
- Why model size and quantization matter on microcontrollers
- TensorFlow Lite for Microcontrollers (TFLM) overview

---

## Exercise 1.1 — Measure Raw Camera Throughput

Before adding any AI, establish a baseline: how fast can the ESP32 capture and encode frames?

```c
// firmware/lab_01/main/main.c
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "lab01";

void app_main(void) {
    camera_config_t config = CAMERA_CONFIG_DEFAULT();  // defined in camera_pins.h
    config.frame_size = FRAMESIZE_QVGA;  // 320x240
    config.pixel_format = PIXFORMAT_GRAYSCALE;
    ESP_ERROR_CHECK(esp_camera_init(&config));

    int frames = 0;
    int64_t start = esp_timer_get_time();
    while (frames < 100) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
            esp_camera_fb_return(fb);
            frames++;
        }
    }
    int64_t elapsed_us = esp_timer_get_time() - start;
    ESP_LOGI(TAG, "100 frames in %.2f s = %.1f FPS",
             elapsed_us / 1e6, 100.0 / (elapsed_us / 1e6));
}
```

**Expected results (OV3660, ESP32-S3 @ 240 MHz):**

| Config | FPS | Avg frame |
|---|---|---|
| QQVGA grayscale (160×120) | ~22 | ~19 KB |
| QVGA grayscale (320×240) | ~11 | ~75 KB |
| QVGA JPEG (320×240) | ~28 | ~2 KB |
| VGA JPEG (640×480) | ~26 | ~8 KB |

Notice JPEG outperforms raw grayscale — smaller DMA transfer wins over the JPEG encoder overhead. Record your numbers and compare after adding inference in Lab 2.

---

## Exercise 1.2 — Inspect Model Size Constraints

The ESP32-S3-WROOM N16R8 has:
- 16 MB flash (model lives here, read-only)
- 8 MB PSRAM (model weights + activations + camera buffer share this)
- ~512 KB internal SRAM (stack, FreeRTOS, drivers)

Run the model size calculator:

```bash
cd firmware/tools
python3 model_budget.py \
  --psram_kb 8192 \
  --camera_res 320x240 \
  --camera_format grayscale \
  --model_kb 300
```

The script prints whether the model fits alongside the camera frame buffer and what's left for activations.

**Key insight:** A MobileNetV1 0.25× quantized to INT8 at 96×96 input is ~300 KB — fits comfortably. A full MobileNetV2 at 224×224 does not.

---

## Exercise 1.3 — Preview the Completed Inference Pipeline

Before implementing the pipeline yourself in Lab 2, flash the completed
`lab_02` firmware and observe what the finished system looks like.

```bash
# One-time: generate the model C array (downloads ~300 KB .tflite)
bash firmware/tools/fetch_model.sh

# Build and flash
bash firmware/tools/build.sh lab_02 flash
```

Then open the serial monitor:
```bash
bash firmware/tools/build.sh lab_02 monitor
```

You should see output like:
```
  [   0]  no person     score=  54  |  prep=1ms  infer=382ms  total=383ms
  [   1]  no person     score=  47  |  prep=1ms  infer=384ms  total=385ms
  [   2]  person        score=  81  |  prep=1ms  infer=382ms  total=383ms
```

> **No hardware?** The firmware falls back to simulation mode automatically
> (see the Wokwi section in Lab 2). Run the Wokwi simulator and you'll see
> the same output tagged `[SIM]`.

**Discussion questions:**
1. How many milliseconds does a single inference take?
2. What is the end-to-end FPS vs your Lab 1.1 camera-only baseline?
3. Preprocessing takes ~1 ms. Where does the remaining time go?

---

## Checkpoint

- [ ] Camera throughput measured and recorded for all four configurations
- [ ] Model budget script run and budget understood
- [ ] lab_02 firmware flashed and inference output observed live
- [ ] Can explain the camera buffer / PSRAM tradeoff in your own words
