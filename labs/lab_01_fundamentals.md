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

> **No hardware?** Skip to exercise 1.2 and use the reference numbers in the
> table below — you'll validate the inference side in exercise 1.3 with Wokwi.

Before adding any AI, establish a baseline: how fast can the ESP32 capture and encode frames?

Build and flash `lab_01`, then open the monitor:
```bash
bash firmware/tools/build.sh lab_01 flash
bash firmware/tools/build.sh lab_01 monitor
```

The firmware runs all four configurations back-to-back and prints a results table. The core of the benchmark loop *(simplified — the actual firmware in `firmware/lab_01/main/main.c` iterates four configurations)*:

```c
camera_config_t cfg = {
    .pixel_format = PIXFORMAT_GRAYSCALE,
    .frame_size   = FRAMESIZE_QVGA,   // 320×240
    .fb_count     = 2,
    .grab_mode    = CAMERA_GRAB_LATEST,
    // ... pin assignments from camera_pins.h
};
ESP_ERROR_CHECK(esp_camera_init(&cfg));

int64_t t0 = esp_timer_get_time();
for (int i = 0; i < 100; i++) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb) esp_camera_fb_return(fb);
}
int64_t elapsed_us = esp_timer_get_time() - t0;
printf("100 frames in %.2f s = %.1f FPS\n",
       elapsed_us / 1e6, 100.0 / (elapsed_us / 1e6));
```

**Expected results (OV3660, ESP32-S3 @ 240 MHz):**

| Config | FPS (reference) | Avg frame |
|---|---|---|
| QQVGA grayscale (160×120) | 22.2 | ~19 KB |
| QVGA grayscale (320×240) | 11.1 | ~75 KB |
| QVGA JPEG (320×240) | 27.8 | ~2 KB |
| VGA JPEG (640×480) | 26.4 | ~8 KB |

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

> **Note on tensor arena:** Without `--arena_kb`, the script estimates the
> arena as 2× the model size (600 KB). The actual arena in `lab_02` is 120 KB
> (`kArenaSize` in `inference.cc`). Add `--arena_kb 120` for an accurate
> budget; the 2× default is deliberately conservative for planning purposes.

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

**Hardware:** open the serial monitor:
```bash
bash firmware/tools/build.sh lab_02 monitor
```

You should see output like (scores vary with what the camera sees):
```
  [   0]  no person     score=  23  |  prep=1ms  infer=382ms  total=383ms
  [   1]  no person     score= -12  |  prep=1ms  infer=382ms  total=383ms
  [   2]  person        score=  67  |  prep=1ms  infer=382ms  total=383ms
```

**No hardware — Wokwi path:**

If you haven't already done this in Lab 0:
```bash
bash firmware/tools/fetch_model.sh
bash firmware/tools/build.sh lab_02 build
```
Open the `firmware/lab_02/` folder in VS Code and press **F1 → Wokwi: Start
Simulator**. The serial panel will show inference output tagged `[SIM]`:
```
W (500) lab_02: Camera init failed — entering simulation mode
  [   0]  no person     score=  54  |  prep=1ms  infer=382ms  total=383ms  [SIM]
  [   1]  no person     score=  47  |  prep=1ms  infer=382ms  total=383ms  [SIM]
```
The `[SIM]` scores are deterministic (synthetic frames). The inference latency
is identical to hardware — only the camera capture step differs.

**Discussion questions:**
1. How many milliseconds does a single inference take?
2. What is the end-to-end FPS vs your Lab 1.1 camera-only baseline?
3. Preprocessing takes ~1 ms. Where does the remaining time go?

---

## Checkpoint

- [ ] Model budget script run and budget understood
- [ ] Can explain the camera buffer / PSRAM tradeoff in your own words
- [ ] Inference output observed (hardware: live serial; Wokwi: `[SIM]` output)
- [ ] **Hardware only:** camera throughput measured for all four configurations
