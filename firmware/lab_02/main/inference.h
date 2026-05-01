#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int   class_index;  // index into labels array
    int8_t score;       // raw INT8 score in [-128, 127]; 127 = max confidence
} inference_result_t;

// Load model and allocate tensor arena (in PSRAM).
// Call once at startup. Panics on failure.
void inference_init(const uint8_t *model_data, size_t model_len,
                    int input_w, int input_h);

// Run one inference pass.
// frame: INT8 grayscale pixels, input_w * input_h bytes
// Returns the winning class and its raw score.
inference_result_t inference_run(const int8_t *frame);

#ifdef __cplusplus
}
#endif
