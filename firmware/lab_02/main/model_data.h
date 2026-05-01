#pragma once

#include <cstddef>
#include <cstdint>

// Person detection model — MobileNetV1 0.25× 96×96 INT8
// Generated from models/person_detect.tflite via:
//   xxd -i models/person_detect.tflite > main/model_data.cc
// Run firmware/tools/fetch_model.sh to download and regenerate.
extern const unsigned char person_detect_tflite[];
extern const unsigned int  person_detect_tflite_len;
