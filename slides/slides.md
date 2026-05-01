## Slide 1 — Embedded Vision AI on ESP32

- **Embedded Vision AI on ESP32**
- Building Real-Time Edge Intelligence with Microcontrollers
- Presented by: [Presenter Name]
- Packt Live Online Workshop — [Date]
- All code at: `github.com/AdamPippert/ESP32-VisionAI-Workshop`

> **Speaker note:** Welcome everyone. Today we deploy a real machine-learning vision model on a $15 microcontroller with no cloud, no server, and no internet connection required after setup. By the end of this session you will have done it yourself.

---

## Slide 2 — What We're Building

- **Goal:** Run person-detection inference entirely on an ESP32-S3 microcontroller
- Camera captures QVGA frames → ESP32 preprocesses to 96×96 grayscale
- TensorFlow Lite Micro model infers "person" or "no person" in ~382 ms
- Result printed to serial console; optional LED indicator
- No cloud, no internet, no Raspberry Pi — just the chip

> **Speaker note:** This is a full end-to-end pipeline. You will wire the model in, write the preprocessing code, and watch the serial output change when you move your face in front of the camera. Every piece runs inside a single $15 module.

---

## Slide 3 — Agenda at a Glance

| Time | Segment |
|------|---------|
| 0:00–0:30 | Environment setup & hardware check (Section 1) |
| 0:30–2:00 | Part 1 — Embedded Vision Fundamentals (Section 2) |
| 2:00–2:30 | Break |
| 2:30–4:30 | Part 2 — Deploying an AI Vision Model (Section 3) |
| 4:30–5:00 | Wrap-up, Q&A, next steps (Section 4) |

- Labs are embedded throughout each section
- Checkpoint slides signal when to pause and verify before continuing
- Ask questions in chat; we surface them at each checkpoint

> **Speaker note:** We have four hours of content plus a half-hour break. If you get stuck at a lab step, flag it in chat and keep moving — the reference solution branch is always available.

---

## Slide 4 — Hardware

- **Board:** ESP32-S3-WROOM-1 N16R8 (ASIN B0GDFCCP2G, ~$15)
- **Camera:** OV3660 module (included in kit)
- 16 MB SPI Flash + 8 MB PSRAM (octal, 80 MHz)
- Dual-core Xtensa LX7 @ up to 240 MHz; WiFi 802.11 b/g/n + Bluetooth 5
- Dual USB-C: one for power/flash, one for JTAG debug
- Vector extensions for ML integer arithmetic

> **Speaker note:** The N16R8 suffix is the important part: 16 MB flash, 8 MB PSRAM. Earlier S3 variants without PSRAM cannot run this workshop's firmware — the tensor arena alone needs ~120 KB of external RAM.

---

## Slide 5 — Toolchain Install

- **ESP-IDF version:** v5.4.1 (exact version matters)
- Install system dependencies:

```bash
# macOS
brew install cmake ninja python3 git

# Ubuntu/Debian
sudo apt install cmake ninja-build python3 python3-pip git
```

- Clone and install IDF:

```bash
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf && git checkout v5.4.1
./install.sh esp32s3
source ./export.sh          # run this in every new shell
```

- Verify: `idf.py --version` should print `5.4.1`

> **Speaker note:** The `source ./export.sh` step must be repeated in every new terminal session. A common first-day error is opening a new tab and wondering why `idf.py` is not found. Add it to your shell profile if you plan to use IDF regularly.

---

## Slide 6 — Flash the Test Firmware

- Navigate to the camera-test project and build:

```bash
cd firmware/camera_test
idf.py build
idf.py -p $PORT flash monitor
```

- Replace `$PORT` with your serial port (e.g., `/dev/tty.usbmodem*` on macOS, `/dev/ttyUSB0` on Linux)
- Expected serial output:

```
I (1234) camera: Camera init success
I (1240) camera: OV3660 detected
I (1242) wifi: IP address: 192.168.x.x
```

- If you see `camera init failed`: check the ribbon cable seating

> **Speaker note:** The ribbon cable between the OV3660 and the ESP32-S3 board is the most common hardware failure point. If camera init fails, power off, reseat the cable so the contacts face the correct direction, and retry.

---

## Slide 7 — Camera Stream Live

- After successful flash, note the IP address printed to serial
- Open a browser on the same WiFi network:

```
http://192.168.x.x
```

- You should see a live MJPEG stream at approximately 25 FPS
- Confirm the OV3660 is detected: check serial log for sensor name
- Frame format at this stage: JPEG, 320×240 (QVGA)
- If stream does not load: confirm laptop and board are on same subnet

> **Speaker note:** The web streaming server runs on core 1 while the camera driver runs on core 0 via DMA. In the workshop firmware we disable the HTTP server to free memory for the tensor arena, so enjoy the live view now — it disappears once we deploy the inference firmware.

---

## Slide 8 — Environment Verified ✓

- Checkpoint — all four must be green before continuing:
  - [ ] `idf.py build` completes with no errors
  - [ ] Board boots and prints IP address to serial
  - [ ] IP address is reachable from your browser
  - [ ] Live camera stream visible at approximately 25 FPS
- If any item is red: raise your hand / post in chat now
- Reference branch: `git checkout solution/lab_00` if stuck

> **Speaker note:** This is the last infrastructure checkpoint. Everything from here assumes your board streams video. If you are still debugging hardware at this point, pair with a neighbor or use the recorded session replay — the remaining labs are more valuable than troubleshooting cables.

---

## Slide 9 — Cloud AI vs Edge AI

- **Cloud AI path:** sensor → network → cloud GPU → network → response
  - Typical round-trip latency: 150–300 ms (best case)
  - Requires connectivity; exposes raw data to third parties
- **Edge AI path:** sensor → local MCU → response
  - Typical latency: 1–10 ms (excluding slow models)
  - Data never leaves the device; works offline
- Key tradeoffs: cloud wins on model size and accuracy; edge wins on privacy, latency, cost, reliability
- Today's workshop: 382 ms inference (model-bound, not network-bound)

> **Speaker note:** The 382 ms we see today is slow by edge AI standards — it is entirely model-bound, not network-bound. A cloud call would add 150–300 ms on top of the model time. And that is assuming connectivity exists, which it often does not in industrial or rural deployments.

---

## Slide 10 — Why Edge AI Is Growing

- **Privacy:** raw sensor data (video, audio) never leaves the device
- **Latency:** no network round-trip; response in single-digit milliseconds
- **Cost:** no inference API fees; no per-call billing; one-time hardware cost
- **Offline operation:** continues functioning without any connectivity
- Regulatory tailwind: GDPR, HIPAA, and emerging AI-Act rules push data minimization
- Hardware tailwind: new MCUs ship with ML-specific SIMD and dedicated accelerators

> **Speaker note:** The business case for edge AI is often as much about cost and compliance as it is about performance. A camera that sends video to the cloud for analysis has real API costs at scale — thousands of devices sending millions of frames per day becomes expensive very quickly.

---

## Slide 11 — The ESP32 Family

| Variant | Best For | Key Differentiator |
|---------|----------|--------------------|
| **ESP32-S3** | AI / camera / USB | PSRAM, vector extensions, camera IF |
| **ESP32-C3** | BLE sensors, low power | RISC-V, small form factor |
| **ESP32 (original)** | Legacy / WiFi IoT | No PSRAM, no vector ext |
| **ESP32-S2** | USB HID, security | Single-core, USB OTG |

- Choose S3 when: you need a camera, PSRAM, or ML inference
- Choose C3 when: battery life is the primary concern
- This workshop: S3 only — the PSRAM is non-negotiable for the tensor arena

> **Speaker note:** Espressif has six or seven chip variants now. Do not let the naming confuse you. If someone asks "can I run this on my ESP32?", the answer is: probably not, because the original ESP32 lacks PSRAM and vector extensions. The S3 is the AI-first chip in the family.

---

## Slide 12 — ESP32-S3 Internals

- **CPU:** Dual Xtensa LX7 @ up to 240 MHz (both cores available to FreeRTOS)
- **Internal SRAM:** 512 KB — stack, FreeRTOS, small buffers only
- **PSRAM interface:** up to 8 MB via octal SPI @ 80 MHz (~640 MB/s)
- **Camera parallel interface:** DVP up to 80 MHz, DMA-driven, zero CPU copying
- **Vector extensions:** 128-bit SIMD for integer multiply-accumulate (INT8/INT16)
- **Peripherals:** SPI, I2C, UART, PWM, ADC, USB OTG, JTAG

> **Speaker note:** The vector extensions are what make INT8 inference viable on this chip. Without them, each multiply-accumulate in the convolutional layers would be a scalar operation and inference would be 3–5× slower. ESP-NN, which we cover in Slide 27, uses these extensions automatically.

---

## Slide 13 — Memory Budget

| Tier | Size | Contents |
|------|------|---------|
| **Flash** | 16 MB | Model weights (read-only), firmware code, NVS |
| **PSRAM** | 8 MB | Frame buffers, tensor arena, heap |
| **Internal SRAM** | 512 KB | FreeRTOS stack, ISR handlers, small caches |

- Model (person detection INT8): ~300 KB → flash
- QVGA frame buffer (320×240 gray): 76.8 KB → PSRAM
- Tensor arena (120 KB allocated, ~80 KB used): → PSRAM
- FreeRTOS + app stack: ~80 KB → internal SRAM
- Rule: if it is large and persistent, it lives in PSRAM

> **Speaker note:** The single most common crash in embedded ML is putting the tensor arena in internal SRAM. It fits in the allocator until FreeRTOS stacks and interrupts fill up, then you get a mysterious LoadProhibited exception. Always allocate the arena with `MALLOC_CAP_SPIRAM`.

---

## Slide 14 — Power Envelope

- ESP32-S3 at 240 MHz + active inference: ~100 mA @ 3.3 V (~330 mW)
- On a 1000 mAh 3.7V LiPo: approximately 4 hours of continuous inference
- Comparison: Raspberry Pi 4 + camera + WiFi inference API: ~500–800 mA
- Edge device uses 5–8× less power than equivalent Pi-based cloud solution
- Sleep mode between inferences (light sleep, ~1 cycle/second): ~8–12 mA
- Optimizing FPS vs. power: fewer inferences per second → longer battery life

> **Speaker note:** Four hours of continuous inference sounds short, but most real applications do not run inference 2.6 times per second continuously. A motion-triggered system that wakes on PIR input and runs 3–5 inference cycles might run for weeks on the same battery.

---

## Slide 15 — Real-World Applications

- **Smart doorbell:** face detection locally, only sends notification (not video) to cloud
- **Predictive maintenance:** anomaly detection on conveyor belt vibration + camera, offline factory
- **Robot obstacle avoidance:** <10 ms latency required, cloud is physically impossible
- **Retail shelf monitoring:** out-of-stock detection, store WiFi unreliable, privacy-sensitive
- Common pattern: classify/detect on-device → send lightweight event (label + confidence) → cloud
- The cloud receives intent, not raw sensor data

> **Speaker note:** Notice that in all four examples the compelling reason for edge is different: privacy for the doorbell, connectivity for the factory, latency for the robot, and a mix of connectivity and privacy for the retail shelf. Edge AI is not a single use case — it is a different architectural tier.

---

## Slide 16 — The Vision Pipeline

Five stages, left to right, with approximate time budget:

```
CAPTURE      PREPROCESS    INFER        DECODE       ACT
(camera DMA) (resize+norm) (TFLM)       (argmax)     (serial/GPIO)
   ~0 ms  →    ~1 ms    →  ~382 ms  →   <0.1 ms  →   <0.1 ms
```

- **CAPTURE:** `esp_camera_fb_get()` returns DMA buffer; `GRAB_LATEST` skips stale frames
- **PREPROCESS:** nearest-neighbor downsample 320×240 → 96×96; normalize uint8 → int8
- **INFER:** `interpreter->Invoke()` — blocking, runs all conv layers sequentially
- **DECODE:** argmax over 2 output scores; map index to label string
- **ACT:** `printf()` to UART; optionally `gpio_set_level()` for LED

> **Speaker note:** The pipeline is deliberately synchronous and single-threaded in the workshop firmware to keep the code readable. In production you would run capture and preprocessing on one core while inference runs on the other, which can improve effective throughput.

---

## Slide 17 — Camera Sensor Basics

- **OV3660:** 3 MP max resolution; workshop uses QVGA (320×240)
- Pixel format options and tradeoffs:

| Format | Size (QVGA) | Notes |
|--------|-------------|-------|
| JPEG | ~8–20 KB | Smallest DMA; requires decode step |
| RGB565 | 150 KB | 16-bit color; easy to process |
| GRAYSCALE | 76.8 KB | 8-bit luminance; ideal for inference |

- We use GRAYSCALE: no decode step, smallest buffer, model trained on grayscale
- Camera driver handles format conversion in hardware before DMA transfer

> **Speaker note:** JPEG is tempting because it transfers faster over DMA. But it requires a software decode step back to raw pixels before you can normalize and feed the model. For our pipeline, capturing directly as grayscale saves that decode time and simplifies the preprocessing code.

---

## Slide 18 — Why Grayscale?

- **Input size comparison at 96×96:**
  - RGB: 96 × 96 × 3 = **27,648 bytes**
  - Grayscale: 96 × 96 × 1 = **9,216 bytes** (3× smaller)
- Person detection model was trained on grayscale input — no accuracy penalty
- Human presence is detected by shape and motion silhouette, not color
- Luminance formula (if converting from color): `Y = 0.299R + 0.587G + 0.114B`
- Capturing as `PIXFORMAT_GRAYSCALE` directly avoids the conversion CPU cost

> **Speaker note:** The factor-of-three saving is significant at this memory tier. When your tensor arena is 120 KB and your PSRAM is 8 MB, every kilobyte of input tensor matters. Grayscale is the right choice for any model that was trained on grayscale, which includes the canonical person detection model from TFLM.

---

## Slide 19 — Resizing & Normalization

- **Nearest-neighbor downsample** from 320×240 to 96×96:

```c
for (int y = 0; y < 96; y++) {
    int src_y = y * 240 / 96;
    for (int x = 0; x < 96; x++) {
        int src_x = x * 320 / 96;
        out[y * 96 + x] = src[src_y * 320 + src_x];
    }
}
```

- **Normalize** from `uint8 [0, 255]` to `int8 [-128, 127]`:

```c
int8_t normalized = (int8_t)((int)pixel - 128);
```

- Both operations combined: ~1 ms on hardware
- Bilinear interpolation gives better quality but costs ~3–4 ms — not worth it here

> **Speaker note:** Nearest-neighbor looks crude compared to bilinear, but at 96×96 the quality difference is visually irrelevant for person detection. The model never learned high-frequency texture detail at this resolution anyway. Save the compute for inference.

---

## Slide 20 — What a CNN "Sees"

- Convolutional layers learn hierarchical features, bottom-up:
  - **Layer 1:** edges, gradients — horizontal, vertical, diagonal 5×5 filters
  - **Layer 3–5:** textures, corners, simple shapes
  - **Layer 10+:** object parts — limbs, heads, torsos
  - **Final layer:** class scores combining all detected parts
- Each filter is a small matrix of INT8 weights learned during training
- Our model processes all of this in 382 ms across ~28 layers
- Visualization: activations from early layers look like edge-detected images

> **Speaker note:** If you want to build intuition for what each layer is doing, the "CNN Explainer" web app from Georgia Tech is an excellent interactive tool. I recommend looking at it after the workshop — it shows the exact filter activations in a MobileNet-style network.

---

## Slide 21 — MobileNet Architecture

- **Standard convolution cost:** `K² × C_in × C_out × H × W` multiply-accumulates
- **MobileNet depthwise separable convolution — two steps:**
  - Depthwise: `K² × C_in × H × W` (one filter per input channel)
  - Pointwise: `C_in × C_out × H × W` (1×1 conv to mix channels)
- Combined cost: `K² × C_in × H × W + C_in × C_out × H × W`
- **Reduction factor:** `1/C_out + 1/K²` ≈ **8–9× fewer operations** for K=3, C_out=32
- Same representational power; dramatically lower compute
- α (width multiplier) scales C_in and C_out uniformly: α=0.25 → 16× fewer MACs than α=1.0

> **Speaker note:** This is the core architectural insight that makes inference on a microcontroller possible at all. Howard et al. published MobileNets in 2017, and it is still the foundation of most efficient on-device vision models. The depthwise-then-pointwise pattern shows up in EfficientNet, MobileNetV2, and V3 as well.

---

## Slide 22 — Model Size vs Accuracy

| Model | INT8 Size | Top-1 Accuracy | Inference (our board) |
|-------|-----------|----------------|----------------------|
| MobileNetV1 α=0.25, 96×96 | ~300 KB | ~65% | ~382 ms |
| MobileNetV1 α=0.50, 96×96 | ~800 KB | ~72% | ~780 ms |
| MobileNetV1 α=1.00, 96×96 | ~4.0 MB | ~76% | ~3,200 ms |

- Workshop model: α=0.25 — best speed/size; acceptable accuracy for binary task
- Binary classification (person / no person) needs less discriminative power than 1000-class ImageNet
- All three fit in 16 MB flash; only α=0.25 gives near-real-time on this hardware

> **Speaker note:** The accuracy numbers here are for ImageNet top-1, which is a much harder 1000-class task. For binary person detection, the effective accuracy of the α=0.25 model is substantially higher than 65%. The model was purpose-trained for this two-class task, not repurposed from ImageNet.

---

## Slide 23 — TFLM vs TFLite vs TensorFlow

| Framework | Runtime Size | Target | Dynamic Alloc | OS Required |
|-----------|-------------|--------|---------------|-------------|
| TensorFlow | 600 MB+ | Training servers | Yes | Yes |
| TFLite | ~1 MB | Mobile / Android | Yes | Yes |
| **TFLM** | **<300 KB** | **Microcontrollers** | **No** | **No** |

- TFLM removes: Python runtime, training ops, GC, file I/O, dynamic memory
- TFLM keeps: inference ops only, static allocation, C++ with no exceptions/RTTI
- Model format is identical: `.tflite` FlatBuffer works on all three
- The same model file you test on your laptop runs byte-for-byte on the ESP32

> **Speaker note:** The key TFLM constraint is no dynamic allocation after `AllocateTensors()`. This is a feature, not a limitation — it means memory usage is deterministic and there are no allocation failures at runtime. You know at startup whether your model fits or not.

---

## Slide 24 — INT8 Quantization

- **Goal:** replace 32-bit floats with 8-bit integers; 4× smaller model, faster arithmetic
- **Quantization formula:**

```
W_q = round(W_float / scale) + zero_point
```

- `scale = (W_max - W_min) / 255.0`
- Dequantization for output: `W_float ≈ (W_q - zero_point) × scale`
- **Accuracy impact:** <1% top-1 drop on MobileNetV1 vs float32
- **Speed impact on ESP32-S3:** integer SIMD ops; vector extensions operate on INT8 natively
- Post-training quantization: done offline with representative calibration dataset

> **Speaker note:** The calibration dataset for post-training quantization does not need to be large — a few hundred representative images are enough to capture the activation ranges. The TFLM person detection model was quantized by the TensorFlow team using COCO images and is already available as a pre-quantized INT8 `.tflite`.

---

## Slide 25 — TFLM Interpreter Lifecycle

```cpp
// One-time setup (in app_main or init task)
const tflite::Model* model = tflite::GetModel(person_detect_tflite);
tflite::MicroMutableOpResolver<6> resolver;
resolver.AddDepthwiseConv2D();
// ... add other ops ...

tflite::MicroInterpreter interpreter(
    model, resolver, tensor_arena, kArenaSize);
interpreter.AllocateTensors();   // static, deterministic

// Per-frame inference loop
TfLiteTensor* input = interpreter.input(0);
memcpy(input->data.int8, preprocessed, 96 * 96);
interpreter.Invoke();            // ~382 ms
TfLiteTensor* output = interpreter.output(0);
```

- After `AllocateTensors()`: no heap allocation ever again
- `Invoke()` is synchronous and blocking — feed the frame and wait

> **Speaker note:** Notice that the interpreter, resolver, and arena are all stack or statically allocated objects. There is no `new` or `malloc` in this code path. TFLM's design philosophy is that embedded systems need deterministic memory, not convenient memory.

---

## Slide 26 — Operator Support

- MobileNetV1 person detection requires exactly 6 op kernels:
  - `DepthwiseConv2D` — the main workhorse (most layers)
  - `Conv2D` — first and last pointwise convolutions
  - `AveragePool2D` — spatial pooling before classifier
  - `Reshape` — flatten tensor before Softmax
  - `Softmax` — output probability distribution
  - `Quantize` — input quantization from camera bytes to INT8
- Using `MicroMutableOpResolver<6>` instead of `AllOpsResolver`:
  - Saves ~80–100 KB of flash (all unused op kernels excluded)
  - Explicit op list also catches model/firmware mismatches at startup

> **Speaker note:** `AllOpsResolver` is convenient for prototyping but wasteful for production. If you upgrade the model and the new model uses an op not in your resolver, you get a clear error at `AllocateTensors()` time rather than a silent bad output at inference time. Explicit resolvers are self-documenting.

---

## Slide 27 — ESP-NN Acceleration

- **ESP-NN:** Espressif's library of SIMD-optimized inference kernels for Xtensa LX7
- Replaces generic TFLM reference kernels for DepthwiseConv2D, Conv2D, etc.
- Uses 128-bit vector registers for 16× INT8 multiply-accumulates per cycle
- Enabled automatically: add `espressif/esp-nn` as IDF component → TFLM picks it up
- Measured speedup on person detection:
  - Without ESP-NN: ~700–900 ms
  - With ESP-NN: **~382 ms** (≈2× speedup)
- No code changes required; the component manager handles everything

> **Speaker note:** This is one of the most important practical points in the workshop. If you have a board that is not an S3 or if you forget to include esp-nn, your inference time roughly doubles. The vector extensions in the LX7 are designed specifically for this kind of INT8 workload.

---

## Slide 28 — Model Embedding Workflow

- Convert `.tflite` binary to a C array using `xxd`:

```bash
xxd -i person_detect.tflite > model_data.cc
```

- Output looks like:

```c
const unsigned char person_detect_tflite[] = { 0x18, 0x00, ... };
unsigned int person_detect_tflite_len = 298312;
```

- Add `extern` linkage so it is accessible across translation units:

```c
extern const unsigned char person_detect_tflite[];
extern const unsigned int person_detect_tflite_len;
```

- Model lives in flash `.rodata` section — accessed via MMU cache, never copied to RAM
- `fetch_model.sh` script automates download + xxd + extern patch in one command

> **Speaker note:** The `extern const` distinction matters in C++. A plain `const` at namespace scope has internal linkage (like `static`), so each translation unit gets its own copy. `extern const` forces a single copy in flash shared across all translation units. The model is 300 KB — you definitely do not want multiple copies.

---

## Slide 29 — Lab 1.1 Walkthrough — Camera Throughput

- **Goal:** measure raw camera frame rate across formats and resolutions
- Build and flash:

```bash
cd firmware/lab_01_camera
idf.py build && idf.py -p $PORT flash monitor
```

- Results from hardware (100-frame average):

| Format | Resolution | FPS |
|--------|------------|-----|
| GRAYSCALE | QQVGA (160×120) | 22 |
| GRAYSCALE | QVGA (320×240) | 11 |
| JPEG | QVGA (320×240) | 28 |
| JPEG | VGA (640×480) | 26 |

- Key insight: DMA limits JPEG less than grayscale at higher resolution
- For inference pipeline: QVGA grayscale at 11 FPS matches our 2.6 FPS inference rate with headroom

> **Speaker note:** Notice that JPEG is faster than grayscale at QVGA because the OV3660 hardware JPEG encoder compresses before DMA transfer, so less data crosses the SPI bus. However, as we discussed, we choose grayscale to avoid the decode step in the inference pipeline.

---

## Slide 30 — Lab 1.2 Walkthrough — Model Budget

- **Goal:** understand memory layout before any inference code
- Run the budget analysis script:

```bash
python3 firmware/tools/model_budget.py \
    --model models/person_detect.tflite \
    --arena 122880 \
    --frame-w 320 --frame-h 240
```

- Example output:

```
Model weights:    298 KB  (flash)
Tensor arena:     120 KB  (PSRAM, 80 KB used + 40 KB margin)
Frame buffer:      75 KB  (PSRAM)
Total PSRAM:      195 KB  /  8192 KB  (2.4% used)
Fits: YES
```

- Key insight: we are using less than 3% of PSRAM — room to grow the model

> **Speaker note:** The budget script is the first thing to run when evaluating a new model for a new board. Before writing a single line of inference code, you need to know whether the model fits in flash and whether the arena fits in RAM. This script makes that check fast and reproducible.

---

## Slide 31 — Lab 1.3 Walkthrough — Person Detection Demo

- **Goal:** run the pre-built inference firmware and observe output
- Build and flash:

```bash
cd firmware/lab_02
idf.py build && idf.py -p $PORT flash monitor
```

- Expected serial output (no person present):

```
[0] no person  score=54  | capture=0ms  prep=1ms  infer=382ms  total=384ms
[1] no person  score=61  | capture=0ms  prep=1ms  infer=382ms  total=383ms
```

- Point camera at your face:

```
[12] person    score=87  | capture=0ms  prep=1ms  infer=382ms  total=384ms
```

- The `capture=0ms` means `GRAB_LATEST` flag — we discard the queued frame and grab fresh

> **Speaker note:** The score is a raw INT8 value, not a probability. 87 out of 127 maximum is moderately strong detection. The model does not output softmax probabilities in the traditional sense — it outputs quantized logits. We cover score interpretation in Slide 48.

---

## Slide 32 — Checkpoint / Q&A — Part 1

- Part 1 key constraints to remember:
  - Internal SRAM: 512 KB → stack and FreeRTOS only
  - PSRAM: 8 MB → frame buffers, tensor arena, all large allocations
  - Flash: 16 MB → model weights (read-only) + firmware code
  - Inference time: 382 ms @ 240 MHz with ESP-NN enabled
- **Question for break:** "If you cut the model size in half (α=0.25 → α=0.1), what changes and what does not?"
- Hint: think about which memory tier changes and whether FPS improves linearly

> **Speaker note:** The expected answer: flash usage drops, inference time roughly halves (to ~170 ms), PSRAM usage is nearly unchanged (frame buffer stays the same, tensor arena shrinks slightly). FPS would roughly double — from 2.6 to ~5. Accuracy would drop noticeably.

---

## Slide 33 — Choosing a Pretrained Model

- **Requirements for this workshop:**
  - INT8 quantized (not float32 — no FPU for large tensors)
  - Grayscale input (single channel, reduces input tensor 3×)
  - Fits in 16 MB flash (<16 MB model + firmware overhead)
  - Inference <500 ms for usable demo (TFLM + ESP-NN achievable)
- **Selected model:** `person_detect.tflite` from `tensorflow/tflite-micro`
  - Size: ~300 KB; input: 96×96×1 INT8; output: [no_person, person] INT8
- Sourced from: `github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/person_detection`

> **Speaker note:** This model is the canonical TFLM example and is actively maintained by the TensorFlow team. Using a well-known reference model means your results are reproducible and comparable to published benchmarks. For a production system you would fine-tune on your own data, which we cover in Slide 54.

---

## Slide 34 — Label Set

- Person detection model outputs exactly 2 classes:

| Index | Label | INT8 Range |
|-------|-------|------------|
| 0 | `no_person` | -128 to 127 |
| 1 | `person` | -128 to 127 |

- Stored in `models/labels.txt` (one label per line)
- Output tensor shape: `[1, 2]` type `kTfLiteInt8`
- Winner: class index with the **higher** INT8 score
- The two scores are not independent probabilities; higher one wins regardless of absolute value
- Score of -128 = model is maximally confident this class is NOT present

> **Speaker note:** Students often expect softmax probabilities that sum to 1.0. TFLM outputs quantized logits — the relative ordering matters, not the absolute values. A score of 54 for "no person" and -12 for "person" means no person detected; the gap is what matters.

---

## Slide 35 — Model File to C Array

- Automated with the provided script:

```bash
bash firmware/tools/fetch_model.sh
```

- Script does:
  1. Downloads `person_detect.tflite` from TFLM GitHub release
  2. Runs `xxd -i` to generate raw C array
  3. Applies `sed` to add `extern const` linkage specifiers
  4. Writes `firmware/lab_02/main/model_data.cc` and `model_data.h`

- Variable names in the generated file:
  - `extern const unsigned char person_detect_tflite[]`
  - `extern const unsigned int person_detect_tflite_len`

- Idempotent: safe to run multiple times; skips download if file exists

> **Speaker note:** The fetch script is an example of the deterministic automation principle — instead of explaining a four-step manual process that students must repeat correctly, we encode it in a script that is always correct. Run it once, get the right output every time.

---

## Slide 36 — CMake Integration

- `CMakeLists.txt` for `lab_02/main`:

```cmake
idf_component_register(
    SRCS "app_main.cc" "inference.cc" "model_data.cc" "preprocess.cc"
    INCLUDE_DIRS "."
    REQUIRES espressif__esp32-camera espressif__esp-tflite-micro
)
```

- `idf_component.yml` declares versions:

```yaml
dependencies:
  espressif/esp32-camera: "^2.0.9"
  espressif/esp-tflite-micro: "1.3.5"
  espressif/esp-nn: "1.2.3"
```

- IDF Component Manager fetches all dependencies on first `idf.py build`
- No manual library cloning required

> **Speaker note:** The IDF Component Manager is the equivalent of `pip` or `cargo` for ESP-IDF. It downloads pinned versions and handles transitive dependencies. The `esp-tflite-micro` component wraps the official TFLM source tree and patches it for Xtensa, which saves you from maintaining your own TFLM port.

---

## Slide 37 — Tensor Arena Sizing

- The tensor arena is a static byte array that TFLM uses for all activation tensors
- **How to find the right size:**

```cpp
// After AllocateTensors() succeeds:
size_t used = interpreter.arena_used_bytes();
// For person detection: used ≈ 80,000 bytes
```

- Rule of thumb: allocate `used × 1.5` as safety margin → 120 KB
- Must be allocated from PSRAM — too large for 512 KB internal SRAM:

```cpp
uint8_t* tensor_arena = (uint8_t*)heap_caps_malloc(
    kArenaSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
```

- If `AllocateTensors()` returns `kTfLiteError`: arena too small — increase `kArenaSize`

> **Speaker note:** The exact arena requirement depends on the model architecture and TFLM version. Always check `arena_used_bytes()` after `AllocateTensors()` and log it during development. If you update the TFLM version, the requirement can change slightly.

---

## Slide 38 — Resolver Ops List

- For MobileNetV1 person detection — minimal resolver:

```cpp
tflite::MicroMutableOpResolver<6> resolver;
resolver.AddDepthwiseConv2D();
resolver.AddConv2D();
resolver.AddAveragePool2D();
resolver.AddReshape();
resolver.AddSoftmax();
resolver.AddQuantize();
```

- Template parameter `<6>` = exact number of ops; compile-time bounds check
- Contrast with `AllOpsResolver`: links 40+ kernels, wastes ~80–100 KB flash
- Startup validates model op list against resolver — catches model/firmware mismatches early
- Adding an unneeded op: no harm; missing a needed op: `AllocateTensors()` fails with clear error

> **Speaker note:** Getting the op list wrong is one of the most common TFLM porting mistakes. The error message is clear: "Op type not found in resolver." Run the model through Netron (netron.app) to see every operator in the graph before writing your resolver.

---

## Slide 39 — Input Tensor Layout

- Access the input tensor after `AllocateTensors()`:

```cpp
TfLiteTensor* input = interpreter->input(0);
// Verify layout assumptions:
// input->dims->data = [1, 96, 96, 1]  (NHWC format)
// input->type = kTfLiteInt8
```

- Copy preprocessed frame into input tensor:

```cpp
memcpy(input->data.int8, preprocessed_frame, 96 * 96 * 1);
```

- Layout is NHWC (batch, height, width, channels) — row-major, C-contiguous
- N=1 (single frame per inference), H=96, W=96, C=1 (grayscale)
- Assert dims at startup; do not assume — catches model version mismatches

> **Speaker note:** NHWC vs NCHW is a common source of silent wrong outputs. TFLite uses NHWC. If your preprocessing produces NCHW layout by accident, the scores will be wrong and the model will appear to always output the same class. Always print the dims tensor during development.

---

## Slide 40 — Preprocessing Code

Side-by-side transformation:

```c
// Input: camera frame — uint8[240][320], grayscale
// Output: model input — int8[96][96]

void preprocess_frame(const uint8_t* src, int8_t* dst) {
    for (int y = 0; y < 96; y++) {
        int src_y = y * 240 / 96;
        for (int x = 0; x < 96; x++) {
            int src_x = x * 320 / 96;
            // Nearest-neighbor sample + normalize to [-128, 127]
            dst[y * 96 + x] =
                (int8_t)((int)src[src_y * 320 + src_x] - 128);
        }
    }
}
```

- No floating point, no division in the inner loop
- `- 128` shifts uint8 [0, 255] to int8 [-128, 127] without scaling
- Measured time on hardware: **~1 ms** for 96×96 output

> **Speaker note:** The `-128` normalization works because the model was quantized with zero_point=128 for the input layer. This is a convention used by the TFLM person detection model specifically. Always check the model's input quantization parameters with `input->params.zero_point` if using a different model.

---

## Slide 41 — Invoke and Check Status

```cpp
TfLiteStatus invoke_status = interpreter->Invoke();

if (invoke_status != kTfLiteOk) {
    ESP_LOGE(TAG, "Invoke failed — check arena size and op list");
    return;
}
// If we reach here: inference completed successfully
// Time elapsed: ~382 ms
```

- `Invoke()` is synchronous and blocking — the calling task sleeps while ops execute
- FreeRTOS watchdog: ensure WDT timeout > 382 ms (set in `sdkconfig`)
- If `Invoke()` fails: usually means arena overflow or missing op in resolver
- No partial results — either all ops succeed or the call fails atomically

> **Speaker note:** The FreeRTOS task watchdog defaults to 5 seconds, so 382 ms is safe. However, if you increase model size and inference climbs above 5 seconds, you will get a watchdog reset. Configure `CONFIG_ESP_TASK_WDT_TIMEOUT_S` in `sdkconfig` accordingly.

---

## Slide 42 — Output Tensor Layout

- Access output after `Invoke()` returns `kTfLiteOk`:

```cpp
TfLiteTensor* output = interpreter->output(0);
// output->dims->data = [1, 2]
// output->type = kTfLiteInt8
int8_t no_person_score = output->data.int8[0];
int8_t person_score    = output->data.int8[1];
```

- Score range: -128 (minimum confidence) to 127 (maximum confidence)
- Scores are **not** probabilities; they do not sum to 1
- A score of 0 for both classes means the model sees an ambiguous input
- Winning class: whichever index has the higher INT8 value

> **Speaker note:** Students sometimes add the two scores together expecting 255 or 100%. These are quantized logits, not softmax probabilities. Even after softmax, the TFLM output is then requantized back to INT8 to keep the pipeline in integer arithmetic throughout.

---

## Slide 43 — Argmax + Label Lookup

```cpp
const char* LABELS[] = {"no_person", "person"};
const int   N_LABELS = 2;

// Argmax over output scores
int best_index = 0;
for (int i = 1; i < N_LABELS; i++) {
    if (output->data.int8[i] > output->data.int8[best_index]) {
        best_index = i;
    }
}

const char* label = LABELS[best_index];
int8_t      score = output->data.int8[best_index];

ESP_LOGI(TAG, "[%d] %s  score=%d", frame_count, label, score);
```

- For N_LABELS=2, argmax is just a comparison; loop generalizes to N classes
- `score` reported is the winning class's raw INT8 value — useful for debugging
- Consider filtering: require `score > THRESHOLD` (e.g., 20) before acting

> **Speaker note:** The threshold filter is important in production. Without it, the LED blinks "person" whenever the two scores are close, which happens when the scene is ambiguous — a poorly lit room, a cardboard box at the right distance, etc. A threshold of 20 reduces false positives significantly.

---

## Slide 44 — Inference Latency Profiling

```cpp
int64_t t0 = esp_timer_get_time();   // microseconds
esp_camera_fb_get();                  // capture
int64_t t1 = esp_timer_get_time();
preprocess_frame(fb->buf, input_buf); // resize + normalize
int64_t t2 = esp_timer_get_time();
interpreter->Invoke();                // CNN inference
int64_t t3 = esp_timer_get_time();
```

- Workshop hardware measurements (average over 50 frames):

| Stage | Time |
|-------|------|
| Capture (GRAB_LATEST) | 0 ms |
| Preprocess | 1 ms |
| Invoke | 382 ms |
| **Total** | **384 ms** |

- **Effective FPS:** 1000 / 384 ≈ **2.6 FPS**
- Bottleneck is clearly inference — preprocessing optimization gives <0.3% improvement

> **Speaker note:** The 0 ms capture time is because we set `CAMERA_GRAB_LATEST` mode. This discards the queued frame and grabs the most recent frame from DMA. The DMA capture happened in the background during the previous inference cycle — we never wait for the camera.

---

## Slide 45 — Lab 2.1–2.3 Walkthrough

- **Lab 2.1:** Generate model C array

```bash
bash firmware/tools/fetch_model.sh
# Verify: ls -la firmware/lab_02/main/model_data.cc
```

- **Lab 2.2:** Fill in three `// STEP N:` comments in `inference.cc`:
  - STEP 1: Copy preprocessed frame to `input->data.int8`
  - STEP 2: Call `interpreter->Invoke()` and check return status
  - STEP 3: Read output scores and run argmax

- **Lab 2.3:** Implement `preprocess_frame()` in `preprocess.cc`
  - Reference implementation in `preprocess_reference.cc` if stuck

- Build and flash everything:

```bash
bash firmware/tools/build.sh lab_02 flash
```

> **Speaker note:** The three STEP comments in inference.cc are the core learning exercise. Each one maps directly to a concept we covered on slides 39–43. After completing them, the firmware is functionally complete — the remaining labs add the LED output and optional optimizations.

---

## Slide 46 — Common Bugs

| Symptom | Root Cause | Fix |
|---------|-----------|-----|
| `undefined reference to person_detect_tflite` | Missing `extern` in `model_data.cc` | Re-run `fetch_model.sh`; check `extern const` prefix |
| `AllocateTensors() failed` | Arena too small | Increase `kArenaSize`; check `arena_used_bytes()` |
| Scores always near -128 or fixed | Wrong input normalization range | Ensure `pixel - 128`, not `pixel / 255.0 * 2 - 1` |
| `camera: FB_SIZE` warning in log | Truncated JPEG frame grabbed | Add `if (fb->len < 1000) { esp_camera_fb_return(fb); continue; }` |

- All four bugs are encountered by >30% of first-time implementers
- The normalization bug produces plausible-looking output (no crash) but wrong scores
- Test normalization: solid black frame should give strongly negative person score

> **Speaker note:** The normalization bug is the hardest to diagnose because the firmware runs, inference runs, and you get numbers — but the numbers are wrong. If you see person scores that never exceed 10 even with a face in frame, normalization is the first thing to check.

---

## Slide 47 — Lab 2.4 Live — Serial Output Demo

- With completed inference.cc and preprocess.cc, flash and monitor:
- **No person in frame:**

```
[0] no person  score=54  | capture=0ms prep=1ms infer=382ms total=384ms
[1] no person  score=61  | capture=0ms prep=1ms infer=382ms total=384ms
[2] no person  score=48  | capture=0ms prep=1ms infer=382ms total=384ms
```

- **Face visible in frame:**

```
[3] person     score=87  | capture=0ms prep=1ms infer=382ms total=384ms
[4] person     score=92  | capture=0ms prep=1ms infer=382ms total=384ms
```

- Score fluctuates frame-to-frame — consider smoothing over 3–5 frames
- INT8 score 87 = moderately high confidence; 120+ = very high confidence

> **Speaker note:** Have participants try different conditions: cover the camera (should give no person), hold up a printed photo of a face (should give person — the model does not require a live face), point at a blank wall (no person). The printed photo test is particularly convincing for demonstrating how the CNN focuses on pattern rather than "real" faces.

---

## Slide 48 — Reading INT8 Scores

- Interpreting the raw INT8 output value:

| Score | Meaning |
|-------|---------|
| 127 | Maximum model confidence in this class |
| 64 | Moderate confidence |
| 0 | Model sees ambiguous input; near-even split |
| -64 | Moderate confidence this class is NOT present |
| -128 | Maximum confidence this class is NOT present |

- The two class scores move inversely — as "person" score rises, "no person" score falls
- These are **not** probabilities; do not normalize or exponentiate them
- Practical detection threshold: `person_score > 20` works well for typical indoor lighting
- For multi-class models (N > 2): apply argmax first, then threshold the winner's score

> **Speaker note:** One mental model that helps: think of the INT8 score as a signed vote. Positive votes support the class; negative votes oppose it. The class with the highest total vote count wins. The magnitude tells you how decisive the vote was.

---

## Slide 49 — Optional: LED Output Wiring

- Breadboard wiring for three indicator LEDs:
  - **Green LED:** anode → 220 Ω resistor → GPIO 4 → cathode → GND
  - **Red LED:** anode → 220 Ω resistor → GPIO 5 → cathode → GND
  - **Yellow LED:** anode → 220 Ω resistor → GPIO 6 → cathode → GND
- Semantics:
  - Green: class index == 1 (person detected)
  - Red: class index == 0 (no person)
  - Yellow: `abs(winning_score) < THRESHOLD` (uncertain)
- ESP32-S3 GPIO outputs 3.3 V; 220 Ω limits current to ~15 mA per LED (safe maximum: 40 mA)

> **Speaker note:** The yellow "uncertain" LED is the most interesting one pedagogically. It lights up when the model sees something partially face-shaped — a hand held at a distance, a printed face held sideways, a mannequin head. It makes the model's uncertainty visible, which is a great conversation starter about confidence thresholds.

---

## Slide 50 — Lab 2.5 Code — LED Output

```c
#define TARGET_CLASS      1        // index of "person" in labels
#define CONFIDENCE_THRESH 20       // INT8 threshold for "certain"

#define LED_PERSON_GPIO    4
#define LED_NO_PERSON_GPIO 5
#define LED_UNCERTAIN_GPIO 6

void update_leds(int class_index, int8_t score) {
    bool certain = (score > CONFIDENCE_THRESH);
    gpio_set_level(LED_UNCERTAIN_GPIO, !certain ? 1 : 0);
    gpio_set_level(LED_PERSON_GPIO,
                   (certain && class_index == TARGET_CLASS) ? 1 : 0);
    gpio_set_level(LED_NO_PERSON_GPIO,
                   (certain && class_index != TARGET_CLASS) ? 1 : 0);
}
```

- Add `gpio_config()` calls for all three pins in `app_main()` before inference loop
- Mode: `GPIO_MODE_OUTPUT`; pull: none; interrupt: none

> **Speaker note:** This is the physical output that makes the project feel real. When you implement it and the green LED lights up as you move your face in front of the camera, the abstract concept of edge inference becomes tangible. That physical feedback is worth the extra wiring.

---

## Slide 51 — FPS Breakdown

- Time budget for one inference cycle (384 ms total):

| Stage | Time | Percentage |
|-------|------|-----------|
| Invoke (CNN inference) | 382 ms | 99.5% |
| Preprocessing (resize+norm) | 1 ms | 0.26% |
| Capture (GRAB_LATEST) | 0 ms | ~0% |
| FreeRTOS yield + overhead | 1 ms | 0.26% |

- **Lesson:** optimizing preprocessing has essentially zero impact on FPS
- To double FPS: halve inference time (smaller model or dual-core pipeline)
- To quadruple FPS: cut input resolution from 96×96 to 64×64 (~170 ms) — with accuracy tradeoff

> **Speaker note:** This pie chart is a classic Amdahl's Law demonstration. No matter how much you optimize the 1 ms preprocessing step — even if you reduce it to 0 — you gain less than 0.3% FPS improvement. Every optimization effort should target the 99.5% first.

---

## Slide 52 — Tuning for Speed

| Option | Inference Time | Accuracy Impact | Notes |
|--------|---------------|----------------|-------|
| A: Smaller input 64×64 | ~170 ms | -15% relative | Halves spatial resolution |
| B: Smaller model α=0.1 | ~150 ms | -8% relative | Very aggressive pruning |
| C: Tensor arena to SRAM | N/A | N/A | Not possible — too large |
| D: Dual-core pipeline | ~382 ms latency, ~5 FPS | No change | Capture on core 1, infer core 0 |

- Best practical option: dual-core pipeline (option D)
  - Inference latency unchanged, but effective throughput doubles because capture overlaps
- Option A + D combined: ~170 ms inference + overlapped capture ≈ 6 FPS

> **Speaker note:** The dual-core pipeline uses `xTaskCreatePinnedToCore()` to pin the capture task to core 1 and the inference task to core 0. A ring buffer or `xQueueSend()` transfers frames between cores. This is production-level firmware architecture and is a good topic for a follow-up workshop.

---

## Slide 53 — Tuning for Accuracy

| Option | Inference Time | Accuracy Gain | Notes |
|--------|---------------|--------------|-------|
| A: Larger model α=0.5 | ~780 ms | +7% relative | Fits in flash; slower |
| B: Higher resolution 128×128 | ~500 ms | +3% relative | More spatial detail |
| C: Custom fine-tuning | Unchanged (same model) | +10–20% domain-specific | Requires training data |
| D: Ensemble 2× model runs | ~764 ms | +2–3% via averaging | Diminishing returns |

- Best accuracy gain for domain tasks: option C (custom fine-tuning)
  - Collect 200–500 images of your specific use case
  - Transfer learning with frozen backbone; only retrain classifier head
  - Post-training quantization → same `fetch_model.sh` workflow to deploy

> **Speaker note:** Fine-tuning is not as intimidating as it sounds. Edge Impulse (slide 54) can train and deploy a custom model in under an hour with their web UI. The trained model exports as a TFLM C library that drops directly into this firmware's CMake structure without any changes to inference.cc.

---

## Slide 54 — Next Steps — Custom Training

- **Option A: Edge Impulse (recommended for beginners)**
  - Browser-based data collection, training, and deployment
  - Free tier supports ESP32-S3 targets
  - Exports TFLM C++ library; drop into `idf_component_register SRCS`
  - URL: `edgeimpulse.com`

- **Option B: TensorFlow 2 / Keras**
  - Fine-tune MobileNetV2 with transfer learning: freeze backbone, train head
  - Post-training quantization: `tf.lite.TFLiteConverter` with `OPTIMIZE_FOR_SIZE`
  - Convert: `tflite_convert` → `xxd -i` → same deployment pipeline
  - Full control; steeper learning curve

- Both paths produce a `.tflite` that is drop-in compatible with this firmware

> **Speaker note:** The key message is that the deployment pipeline you built today is reusable. If you train a custom model tonight — whether for plant disease detection, gesture recognition, or quality control — you run `fetch_model.sh` with your new `.tflite` and flash the same firmware. The inference code does not change.

---

## Slide 55 — Next Steps — Production Firmware

- **OTA updates:** `esp_https_ota` component — download new firmware over WiFi, verify SHA256, reboot into new partition

- **Power management:** light sleep between inference cycles
  - Active: ~100 mA; light sleep: ~8 mA; deep sleep: ~10 µA
  - Wake on timer or external interrupt (PIR sensor) every N seconds

- **Security:** flash encryption + secure boot v2
  - Prevents firmware extraction; ensures only signed images boot
  - Configured via `idf.py menuconfig` → Security features

- **Per-device configuration:** NVS (Non-Volatile Storage)
  - Store WiFi credentials, inference threshold, device ID in flash
  - `nvs_flash_init()` → `nvs_open()` → `nvs_get_str()`

> **Speaker note:** These four features — OTA, power management, secure boot, NVS — are the gap between a workshop demo and a product. Each one is a half-day topic on its own. The Espressif documentation for all four is excellent; search "esp-idf ota example" for working starter code.

---

## Slide 56 — What You Built Today

- **Complete embedded vision AI pipeline on $15 hardware:**
  - OV3660 camera → QVGA grayscale (320×240, 76 KB)
  - Nearest-neighbor downsample → 96×96 normalized INT8 (9 KB)
  - TFLite Micro inference → ~382 ms → "person" / "no person"
  - Serial output + optional three-LED indicator
- No cloud. No internet required. No Raspberry Pi. No monthly API bill.
- Entire system — hardware, firmware, model, inference — cost under $15 and fits on one PCB
- Reproducible: every step is in the repo; build from scratch in under 30 minutes

> **Speaker note:** Take a moment to appreciate what just happened. A machine learning model — trained on a GPU cluster, quantized to 8 bits, embedded into flash — just ran inference on a chip smaller than your thumbnail and correctly identified whether a human was present. That is the state of edge AI today.

---

## Slide 57 — Key Takeaways

- Edge AI eliminates the latency, cost, and privacy tradeoffs of cloud inference
- PSRAM budget determines which models fit — always run the budget check first
- INT8 quantization reduces model size 4× with less than 1% accuracy loss on typical vision tasks
- TFLM uses static allocation — no heap after `AllocateTensors()`; memory is deterministic
- ESP-NN SIMD kernels are essential: without them, inference is 2× slower on LX7
- The deployment pipeline (fetch → xxd → CMake → flash) is reusable for any TFLM-compatible model

> **Speaker note:** If you take only one thing from today: edge AI is not a research curiosity — it is production-ready on $15 hardware using open-source tools. The barrier is knowing the pipeline, which you now do.

---

## Slide 58 — The Embedded AI Skill Stack

Five tiers, each building on the one below:

| Tier | Technology | Covered Today |
|------|-----------|--------------|
| **Application Logic** | preprocess + argmax + act | Yes — inference.cc |
| **Model** | TFLite INT8, MobileNetV1 | Yes — person_detect.tflite |
| **Inference Runtime** | TFLM + ESP-NN | Yes — component manager |
| **Firmware** | ESP-IDF v5.4.1 + FreeRTOS | Yes — CMake, sdkconfig |
| **Hardware** | ESP32-S3 N16R8, OV3660 | Yes — board setup |

- This workshop covered all five tiers end-to-end
- Depth varies by tier — each tier is a multi-day topic on its own
- The stack is composable: swap the model without changing runtime; swap the board without changing the model

> **Speaker note:** Most edge AI courses cover only one or two tiers. Today you touched all five. That breadth means you can now reason about the whole system — which tier a bug lives in, which tier to optimize first, and which tier to change when requirements shift.

---

## Slide 59 — Resources

- **ESP-IDF documentation:** `docs.espressif.com/projects/esp-idf`
- **TFLite for Microcontrollers:** `github.com/tensorflow/tflite-micro`
- **esp-tflite-micro component:** `components.espressif.com/components/espressif/esp-tflite-micro`
- **Edge Impulse:** `edgeimpulse.com` — no-code custom model training for MCUs
- **Netron model visualizer:** `netron.app` — inspect any `.tflite` in the browser
- **This workshop repo:** `github.com/AdamPippert/ESP32-VisionAI-Workshop`
- **Espressif forum:** `esp32.com` — active community, Espressif engineers respond

> **Speaker note:** The Netron model visualizer is underrated. Before you deploy any TFLM model, open it in Netron to see the full op graph, tensor shapes, and quantization parameters. It takes 30 seconds and catches many integration mistakes before they become debugging sessions.

---

## Slide 60 — Q&A + Thank You

- **Thank you for attending!**
- Workshop repo: `github.com/AdamPippert/ESP32-VisionAI-Workshop`
  - All labs, solutions, scripts, and slide source in one place
- **Feedback form:** [link placeholder — add your form URL here]
- **Contact / follow:**
  - Email: [presenter email placeholder]
  - LinkedIn / X: [handle placeholder]
- Questions? — Raise hand or type in chat
- Recording will be available at: [Packt platform link placeholder]

> **Speaker note:** Please fill out the feedback form — it genuinely shapes future workshop content. If there was a section that moved too fast, or a lab that could use a better hint, that information directly improves the next cohort's experience. Thank you again.
