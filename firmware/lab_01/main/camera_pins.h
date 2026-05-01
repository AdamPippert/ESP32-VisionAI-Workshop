#pragma once

// Pin definitions for ESP32-S3-CAM with OV3660
// Based on ESP32-S3-EYE reference schematic (same sensor, same interface)
// Board: ESP32-S3-WROOM N16R8 (ASIN B0GDFCCP2G)
//
// If camera init fails, verify these against your board's silkscreen or schematic.

#define CAM_PIN_PWDN  -1   // no hardware power-down
#define CAM_PIN_RESET -1   // no hardware reset

#define CAM_PIN_XCLK  15
#define CAM_PIN_SIOD   4   // I2C SDA
#define CAM_PIN_SIOC   5   // I2C SCL

#define CAM_PIN_D7    16
#define CAM_PIN_D6    17
#define CAM_PIN_D5    18
#define CAM_PIN_D4    12
#define CAM_PIN_D3    10
#define CAM_PIN_D2     8
#define CAM_PIN_D1     9
#define CAM_PIN_D0    11

#define CAM_PIN_VSYNC  6
#define CAM_PIN_HREF   7
#define CAM_PIN_PCLK  13
