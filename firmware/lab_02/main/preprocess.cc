#include "preprocess.h"
#include <string.h>

// Nearest-neighbor downsample from fb->width × fb->height (PIXFORMAT_GRAYSCALE)
// to target_w × target_h, then normalize uint8 → int8 by subtracting 128.
void preprocess_frame(const camera_fb_t *fb, int8_t *out,
                      int target_w, int target_h)
{
    const uint8_t *src = fb->buf;
    int src_w = fb->width;
    int src_h = fb->height;

    for (int y = 0; y < target_h; y++) {
        int src_y = (y * src_h) / target_h;
        for (int x = 0; x < target_w; x++) {
            int src_x = (x * src_w) / target_w;
            uint8_t pixel = src[src_y * src_w + src_x];
            out[y * target_w + x] = (int8_t)((int)pixel - 128);
        }
    }
}
