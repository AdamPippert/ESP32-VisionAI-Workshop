# Lab 2 — Deploying an AI Vision Model

**Duration:** 120 minutes  
**Goal:** Integrate a pretrained TFLite model into the firmware, flash it, and run real-time object classification with optional LED output.

---

## What You're Building

An ESP32 firmware application that:
1. Captures a frame from the OV3660 camera
2. Resizes and normalizes it to 96×96 grayscale
3. Runs INT8 inference using TensorFlow Lite for Microcontrollers
4. Logs the top prediction and confidence to serial
5. (Optional) Drives an LED high when a target class is detected

---

## Before You Start — Generate the Model File

`model_data.cc` is generated (not committed to git). Run this once before building:

```bash
bash firmware/tools/fetch_model.sh
```

This downloads `person_detect.tflite` (~294 KB) from the TFLM GitHub repository
and generates `firmware/lab_02/main/model_data.cc` via `xxd` with the correct
C symbol names.

---

## Exercise 2.1 — Embed the Model in Firmware

TFLite models are stored as C arrays in flash. The `fetch_model.sh` script you
just ran is equivalent to:

```bash
xxd -i models/person_detect.tflite > main/model_data.cc
# then sed to fix symbol names and add 'extern const' for C++ linkage
```

The resulting `model_data.cc` defines:
```c
extern const unsigned char person_detect_tflite[] = { 0x1c, 0x00, ... };
extern const unsigned int person_detect_tflite_len = 300568;
```

> **Why `extern const`?** In C++, a `const` at namespace scope has internal
> linkage by default — the linker can't see it from other translation units.
> `extern const` makes it visible. Raw `xxd` output omits this; `fetch_model.sh`
> adds it automatically.

The `CMakeLists.txt` in `firmware/lab_02/main/` already includes `model_data.cc` — no changes needed.

---

## Exercise 2.2 — Wire Up the Inference Pipeline

Open `firmware/lab_02/main/inference.cc` and read through the three key steps
implemented in `inference_run()`:

```cpp
// STEP 1: Copy preprocessed frame into the input tensor
TfLiteTensor *input = g_interp->input(0);
// input->dims: [1, 96, 96, 1]  type: kTfLiteInt8
memcpy(input->data.int8, frame, g_input_w * g_input_h);

// STEP 2: Run inference
if (g_interp->Invoke() != kTfLiteOk) {
    ESP_LOGE(TAG, "Invoke() failed");
    return {0, -128};
}

// STEP 3: Argmax over output scores
TfLiteTensor *output = g_interp->output(0);
int num_classes = output->dims->data[output->dims->size - 1];
int8_t *scores  = output->data.int8;
int best = 0;
for (int i = 1; i < num_classes; i++) {
    if (scores[i] > scores[best]) best = i;
}
return {best, scores[best]};
```

**Discussion questions:**
1. Why does `inference_init()` allocate the tensor arena with `MALLOC_CAP_SPIRAM`?
2. The model has 2 output classes. What are they, and how would you add a third?
3. What does `kArenaSize = 120 * 1024` represent, and what happens if it's too small?

---

## Exercise 2.3 — Preprocess Camera Frames

The firmware configures the camera with `PIXFORMAT_GRAYSCALE`, so each frame
buffer contains raw 8-bit luma bytes — no JPEG decode or RGB conversion needed.
The model expects 96×96 INT8. Open `firmware/lab_02/main/preprocess.cc`:

```c
void preprocess_frame(const camera_fb_t *fb, int8_t *out,
                      int target_w, int target_h)
{
    const uint8_t *src = fb->buf;
    int src_w = fb->width;   // 320
    int src_h = fb->height;  // 240

    for (int y = 0; y < target_h; y++) {
        int src_y = (y * src_h) / target_h;   // nearest-neighbor row
        for (int x = 0; x < target_w; x++) {
            int src_x = (x * src_w) / target_w;  // nearest-neighbor col
            uint8_t pixel = src[src_y * src_w + src_x];
            out[y * target_w + x] = (int8_t)((int)pixel - 128);  // uint8→int8
        }
    }
}
```

**Why subtract 128?** The person detection model was trained on INT8 inputs
where the zero point maps to 128 in uint8 space. Subtracting 128 re-centers
the distribution around zero, matching the quantization the model expects.

**Discussion questions:**
1. What is the output resolution and why 96×96?
2. Nearest-neighbor is fast but lossy. What alternative would give better quality?
3. What would break if you passed a JPEG frame buffer directly to this function?

---

## Exercise 2.4 — Build, Flash, and Test

```bash
bash firmware/tools/build.sh lab_02 flash
bash firmware/tools/build.sh lab_02 monitor
```

Watch the serial output. Point the camera at:
- Your face
- A coffee mug
- A plant
- Your keyboard

Note which classes fire and the raw INT8 score. Scores near `127` = high confidence; near `-128` = low.

---

## Exercise 2.5 (Optional) — LED Event Output

Wire three LEDs to GPIO 4, 5, 6 through 220Ω resistors to GND.

In `firmware/lab_02/main/main.cc`, enable the LED driver:

```c
#define LED_CLASS_0_GPIO  4   // e.g., "person detected"
#define LED_CLASS_1_GPIO  5   // e.g., "no person"
#define LED_CLASS_2_GPIO  6   // e.g., "uncertain" (score < threshold)

gpio_set_level(LED_CLASS_0_GPIO, best_class == TARGET_CLASS ? 1 : 0);
```

Set `TARGET_CLASS` to the index of whatever class you want to detect. Labels are in `models/labels.txt`.

---

## Performance Checkpoint

Run the monitor and filter for timing lines:

```bash
bash firmware/tools/build.sh lab_02 monitor | grep "infer="
```

Measured results on ESP32-S3 at 240 MHz (OV3660, TFLM person detection model):
| Resolution | Model | Latency | FPS |
|---|---|---|---|
| 96×96 grayscale | Person detection INT8 | ~382 ms | ~2.6 FPS |

Inference takes ~382ms — preprocessing (1ms) and capture (0ms via GRAB_LATEST) are negligible.
The 2.6 FPS end-to-end figure includes a 1-tick yield to keep the scheduler healthy.

---

## Wokwi Simulation (no hardware required)

The lab_02 firmware detects at runtime whether a camera is present. If camera
initialisation fails — as it will in Wokwi, which has no OV3660 model — the
firmware falls back automatically to synthetic frame generation and continues
running inference.

### One-time setup

1. Install the **Wokwi for VS Code** extension from the VS Code Marketplace.
   Sign in at wokwi.com and follow the extension's **Activate License** prompt.
2. Build the firmware (the `.elf` must exist before Wokwi can run):
   ```bash
   bash firmware/tools/fetch_model.sh   # generate model_data.cc (if not done yet)
   bash firmware/tools/build.sh lab_02 build
   ```
3. Open the **`firmware/lab_02/`** folder in VS Code *(not the repo root —
   Wokwi locates `wokwi.toml` relative to the workspace root)*.
4. Press **F1 → Wokwi: Start Simulator**.

### What you'll see

The serial monitor panel will show:
```
W (500) lab_02: Camera init failed — entering simulation mode
W (500) lab_02: Synthetic frames will be generated (Wokwi / no-camera build)
...
=== SIMULATION MODE — synthetic frames (no camera) ===

  [   0]  no person     score=  54  |  prep=1ms  infer=382ms  total=383ms  [SIM]
  [   1]  no person     score=  47  |  prep=1ms  infer=382ms  total=383ms  [SIM]
```

Each line is tagged `[SIM]` to make the mode visible. The pipeline — preprocess,
TFLM invoke, argmax, score print — runs identically to hardware; only the camera
capture step is replaced by the synthetic frame generator.

The three LEDs in the Wokwi diagram (GPIO 4/5/6) are pre-wired for Exercise 2.5.

---

## Checkpoint

- [ ] `fetch_model.sh` ran and `model_data.cc` exists with `person_detect_tflite[]`
- [ ] `bash firmware/tools/build.sh lab_02 build` completes cleanly
- [ ] Can explain why `extern const` is required for the model array in C++
- [ ] Can trace the three steps in `inference_run()`: copy → invoke → argmax
- [ ] Can explain why grayscale frames need no JPEG decode or RGB conversion
- [ ] Serial output shows class name and score on each frame
- [ ] End-to-end FPS measured and compared to Lab 1.1 baseline
- [ ] (Optional) LED lights when target class detected
