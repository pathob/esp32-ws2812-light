#ifndef __MAIN_LIGHT_H__
#define __MAIN_LIGHT_H__

#include <math.h>

#include "esp_types.h"

typedef struct LIGHT_rgb_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} LIGHT_rgb_t;

void LIGHT_rgb_from_mired(
    uint16_t mired,
    LIGHT_rgb_t *rgb);

void LIGHT_rgb_from_kelvin(
    uint16_t kelvin,
    LIGHT_rgb_t *rgb);

#endif

