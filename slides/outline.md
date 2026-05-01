# Slide Deck Outline — Embedded Vision AI on ESP32

**Target:** 50–60 slides, Packt Live Online format  
**Sections map to workshop agenda:** Setup (slides 1–8), Part 1 (slides 9–32), Part 2 (slides 33–55), Wrap-Up (slides 56–60)

---

## Section 1: Opening & Setup (Slides 1–8)

1. **Title slide** — workshop name, presenter, date
2. **What we're building** — one-sentence goal + photo of finished hardware
3. **Agenda at a glance** — timeline with session names
4. **Hardware check** — photo of ESP32-S3-CAM board, label key components
5. **Toolchain install** — step-by-step (ESP-IDF, esptool, Python)
6. **Flash the test firmware** — commands, expected serial output
7. **Camera stream live** — screenshot of browser MJPEG view
8. **Environment verified ✓** — checkpoint, prompt participants to confirm

---

## Section 2: Embedded Vision Fundamentals (Slides 9–32)

### Edge AI Landscape (slides 9–15)
9. **Cloud AI vs Edge AI** — diagram: request → cloud → response vs. device-local
10. **Why edge AI is growing** — privacy, latency, cost, offline operation
11. **The ESP32 family** — S3 vs C3 vs original, when to choose each
12. **ESP32-S3 internals** — dual Xtensa LX7, 512 KB SRAM, PSRAM interface diagram
13. **Memory budget** — flash / PSRAM / SRAM split, what lives where
14. **Power envelope** — mA draw at inference vs idle; battery life math
15. **Real-world applications** — smart door camera, predictive maintenance sensor, robot vision

### Computer Vision on Microcontrollers (slides 16–22)
16. **The vision pipeline** — capture → decode → preprocess → infer → act
17. **Camera sensor basics** — OV3660 specs, pixel format options, resolution tradeoffs
18. **Why grayscale?** — 3× smaller input, minimal accuracy loss for many tasks
19. **Resizing & normalization** — nearest-neighbor downsample, [0,255]→[-128,127]
20. **What a CNN "sees"** — visualization: filters at conv1 on an ESP32-sized input
21. **MobileNet architecture** — depthwise separable convolutions in plain English
22. **Model size vs accuracy tradeoff** — chart: MobileNetV1 α=0.25/0.5/1.0

### TFLite for Microcontrollers (slides 23–32)
23. **TFLM vs TFLite vs TF** — hierarchy diagram, what gets stripped out
24. **INT8 quantization** — float32 → int8, scale/zero-point math, why accuracy holds
25. **TFLM interpreter lifecycle** — Model → Resolver → Interpreter → Invoke loop
26. **Operator support** — which ops are available on ESP32 (CMSIS-NN, ESP-NN)
27. **ESP-NN acceleration** — Espressif's SIMD kernels, when they kick in
28. **Model embedding workflow** — `.tflite` → `xxd` → C array in flash
29. **Lab 1.1 walkthrough** — camera throughput benchmark code
30. **Lab 1.2 walkthrough** — model budget calculator output
31. **Lab 1.3 walkthrough** — person detection demo, serial output screenshot
32. **Checkpoint / Q&A** — recap key constraints before break

---

## Break (no slide)

---

## Section 3: Deploying an AI Vision Model (Slides 33–55)

### Model Selection & Integration (slides 33–38)
33. **Choosing a pretrained model** — MobileNetV1 0.25× INT8 as the workshop baseline
34. **Label set overview** — 1000 ImageNet classes vs custom training
35. **Model file to C array** — `xxd` command, file layout in `model_data.cc`
36. **CMake integration** — how `model_data.cc` lands in the build
37. **Tensor arena sizing** — how to calculate, what happens if too small
38. **Resolver ops list** — which ops to register for MobileNetV1

### Inference Pipeline Code (slides 39–46)
39. **Input tensor layout** — dims, type, pointer arithmetic
40. **Preprocessing code** — JPEG decode, downsample, normalize (side-by-side C code)
41. **Invoke and check status** — error handling pattern
42. **Output tensor layout** — INT8 scores, dequantization
43. **Argmax + label lookup** — finding best class
44. **Inference latency profiling** — `esp_timer` around Invoke()
45. **Lab 2.1–2.3 walkthrough** — live code demo, key lines highlighted
46. **Common bugs** — wrong tensor type, arena too small, PSRAM not enabled

### Running & Extending (slides 47–55)
47. **Lab 2.4 live** — serial monitor output, pointing camera at objects
48. **Reading INT8 scores** — what 127, 0, -128 mean in practice
49. **Optional: LED output wiring** — breadboard diagram, GPIO assignments
50. **Lab 2.5 code** — gpio_set_level on detection event
51. **FPS breakdown** — capture + preprocess + infer + output, where time goes
52. **Tuning for speed** — lower resolution, smaller model, PSRAM→SRAM for arena
53. **Tuning for accuracy** — larger model, higher resolution, custom fine-tuning path
54. **Next steps: custom training** — Edge Impulse, TF2 → TFLite → ESP32 pipeline overview
55. **Next steps: production firmware** — OTA updates, power management, secure boot

---

## Section 4: Wrap-Up (Slides 56–60)

56. **What you built today** — photo collage of running hardware
57. **Key takeaways** — 5 bullet points, one per learning objective
58. **The embedded AI skill stack** — where this workshop fits in a larger roadmap
59. **Resources** — ESP-IDF docs, TFLM repo, Edge Impulse, Espressif AI examples
60. **Q&A + Thank you**
