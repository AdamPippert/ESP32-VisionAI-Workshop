#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "camera_pins.h"

static const char *TAG = "lab_01";

// ---------------------------------------------------------------------------
// Test matrix: resolutions and formats to benchmark
// ---------------------------------------------------------------------------
typedef struct {
    const char       *label;
    framesize_t       framesize;
    pixformat_t       pixformat;
    const char       *fmt_name;
    int               width;
    int               height;
} bench_config_t;

static const bench_config_t BENCH[] = {
    { "QQVGA grayscale", FRAMESIZE_QQVGA, PIXFORMAT_GRAYSCALE, "grayscale", 160, 120 },
    { "QVGA  grayscale", FRAMESIZE_QVGA,  PIXFORMAT_GRAYSCALE, "grayscale", 320, 240 },
    { "QVGA  JPEG     ", FRAMESIZE_QVGA,  PIXFORMAT_JPEG,      "jpeg",      320, 240 },
    { "VGA   JPEG     ", FRAMESIZE_VGA,   PIXFORMAT_JPEG,      "jpeg",      640, 480 },
};
#define BENCH_COUNT (sizeof(BENCH) / sizeof(BENCH[0]))
#define FRAMES_PER_RUN 100

// ---------------------------------------------------------------------------
// Camera init / reinit
// ---------------------------------------------------------------------------
static esp_err_t camera_init(pixformat_t fmt, framesize_t size)
{
    esp_camera_deinit();   // safe to call even if not yet inited

    camera_config_t cfg = {
        .pin_pwdn     = CAM_PIN_PWDN,
        .pin_reset    = CAM_PIN_RESET,
        .pin_xclk     = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,
        .pin_d7 = CAM_PIN_D7, .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5, .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3, .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1, .pin_d0 = CAM_PIN_D0,
        .pin_vsync    = CAM_PIN_VSYNC,
        .pin_href     = CAM_PIN_HREF,
        .pin_pclk     = CAM_PIN_PCLK,

        .xclk_freq_hz = 20000000,
        .ledc_timer   = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = fmt,
        .frame_size   = size,
        .jpeg_quality = 12,
        .fb_count     = 2,
        .fb_location  = CAMERA_FB_IN_PSRAM,
        .grab_mode    = CAMERA_GRAB_LATEST,
    };

    return esp_camera_init(&cfg);
}

// ---------------------------------------------------------------------------
// Benchmark one configuration
// ---------------------------------------------------------------------------
static void run_benchmark(const bench_config_t *cfg)
{
    ESP_LOGI(TAG, "Setting up: %s (%dx%d)", cfg->label, cfg->width, cfg->height);

    if (camera_init(cfg->pixformat, cfg->framesize) != ESP_OK) {
        ESP_LOGE(TAG, "  Camera init failed — skipping");
        return;
    }

    // Discard first few frames while sensor stabilises
    for (int i = 0; i < 5; i++) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) esp_camera_fb_return(fb);
    }

    // Timed capture loop
    int64_t t0 = esp_timer_get_time();
    int captured = 0;
    size_t total_bytes = 0;

    for (int i = 0; i < FRAMES_PER_RUN; i++) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGW(TAG, "  Frame %d missed", i);
            continue;
        }
        total_bytes += fb->len;
        esp_camera_fb_return(fb);
        captured++;
    }

    int64_t elapsed_us = esp_timer_get_time() - t0;
    float elapsed_s = elapsed_us / 1e6f;
    float fps = captured / elapsed_s;
    float avg_kb = (total_bytes / (float)captured) / 1024.0f;

    printf("\n");
    printf("  %-28s  %5.1f FPS   avg frame: %5.1f KB\n",
           cfg->label, fps, avg_kb);
    printf("  %d frames in %.3f s\n", captured, elapsed_s);
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
void app_main(void)
{
    ESP_LOGI(TAG, "Lab 01 — Camera Throughput Benchmark");
    ESP_LOGI(TAG, "ESP32-S3 @ 240 MHz, PSRAM 8MB octal 80MHz");
    ESP_LOGI(TAG, "Capturing %d frames per config", FRAMES_PER_RUN);

    vTaskDelay(pdMS_TO_TICKS(500));  // let UART settle

    printf("\n");
    printf("============================================================\n");
    printf("  Config                          FPS      Avg frame size\n");
    printf("============================================================\n");

    for (size_t i = 0; i < BENCH_COUNT; i++) {
        run_benchmark(&BENCH[i]);
        vTaskDelay(pdMS_TO_TICKS(200));  // settle between configs
    }

    printf("============================================================\n");
    printf("\n");
    printf("Record your FPS numbers — you will compare them against\n");
    printf("Lab 02 results after adding model inference.\n");
    printf("\n");

    ESP_LOGI(TAG, "Benchmark complete. Reset board to run again.");
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
