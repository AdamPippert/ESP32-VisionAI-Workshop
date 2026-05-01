#pragma once

#include "esp_camera.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Downsample a PIXFORMAT_GRAYSCALE camera frame to target_w × target_h
// and normalize pixel values to INT8 [-128, 127].
// out must point to at least target_w * target_h bytes of writable memory.
void preprocess_frame(const camera_fb_t *fb, int8_t *out,
                      int target_w, int target_h);

#ifdef __cplusplus
}
#endif
