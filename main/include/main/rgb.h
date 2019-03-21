#ifndef __MAIN_RGB_H__
#define __MAIN_RGB_H__

#include <math.h>

#include "esp_types.h"

typedef struct RGB_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB_t;

void RGB_from_mired(
    uint16_t mired,
    RGB_t *rgb);

void RGB_from_kelvin(
    uint16_t kelvin,
    RGB_t *rgb);

#endif