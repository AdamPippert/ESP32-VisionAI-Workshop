#include "inference.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include <inttypes.h>
#include <string.h>

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

static const char *TAG = "inference";

// Ops needed by MobileNetV1 0.25× person detection (INT8)
static tflite::MicroMutableOpResolver<6> g_resolver;

static const tflite::Model  *g_model      = nullptr;
static tflite::MicroInterpreter *g_interp = nullptr;
static uint8_t *g_arena                   = nullptr;
static int      g_input_w                 = 96;
static int      g_input_h                 = 96;

static const int kArenaSize = 120 * 1024;  // 120 KB in PSRAM

void inference_init(const uint8_t *model_data, size_t model_len,
                    int input_w, int input_h)
{
    g_input_w = input_w;
    g_input_h = input_h;

    tflite::InitializeTarget();

    g_model = tflite::GetModel(model_data);
    if (g_model->version() != TFLITE_SCHEMA_VERSION) {
        ESP_LOGE(TAG, "Model schema mismatch: got %" PRIu32 ", expected %d",
                 g_model->version(), TFLITE_SCHEMA_VERSION);
        abort();
    }

    // Register only the ops this model uses
    g_resolver.AddDepthwiseConv2D();
    g_resolver.AddConv2D();
    g_resolver.AddAveragePool2D();
    g_resolver.AddReshape();
    g_resolver.AddSoftmax();
    g_resolver.AddQuantize();

    // Tensor arena lives in PSRAM so it doesn't eat internal SRAM
    g_arena = (uint8_t *)heap_caps_malloc(kArenaSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!g_arena) {
        ESP_LOGE(TAG, "Failed to allocate %d B tensor arena in PSRAM", kArenaSize);
        abort();
    }

    g_interp = new tflite::MicroInterpreter(g_model, g_resolver, g_arena, kArenaSize);
    if (g_interp->AllocateTensors() != kTfLiteOk) {
        ESP_LOGE(TAG, "AllocateTensors() failed");
        abort();
    }

    TfLiteTensor *input = g_interp->input(0);
    ESP_LOGI(TAG, "Input tensor: [%d,%d,%d,%d] type=%d",
             input->dims->data[0], input->dims->data[1],
             input->dims->data[2], input->dims->data[3],
             input->type);
    ESP_LOGI(TAG, "Tensor arena used: %zu B", g_interp->arena_used_bytes());
}

inference_result_t inference_run(const int8_t *frame)
{
    TfLiteTensor *input = g_interp->input(0);
    memcpy(input->data.int8, frame, g_input_w * g_input_h);

    if (g_interp->Invoke() != kTfLiteOk) {
        ESP_LOGE(TAG, "Invoke() failed");
        return {0, -128};
    }

    TfLiteTensor *output = g_interp->output(0);
    int num_classes = output->dims->data[output->dims->size - 1];
    int8_t *scores  = output->data.int8;

    int best = 0;
    for (int i = 1; i < num_classes; i++) {
        if (scores[i] > scores[best]) best = i;
    }

    return {best, scores[best]};
}
