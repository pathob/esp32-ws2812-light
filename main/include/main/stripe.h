#ifndef __MAIN_STRIPE_H__
#define __MAIN_STRIPE_H__

#include <math.h>

#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_types.h"

#include "cJSON.h"

#include "periphery/ws2812.h"

#include "main/defs.h"
#include "main/httpd.h"
#include "main/mqtt.h"
#include "main/rgb.h"

void STRIPE_init();

void STRIPE_set(
    uint8_t *state,
    uint8_t *brightness,
    WS2812_color_t *color);

void STRIPE_set_json(
    char *data);

uint8_t STRIPE_get_state();

void STRIPE_toggle();

#endif
