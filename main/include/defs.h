#ifndef __DEFS_H__
#define __DEFS_H__

#include "esp_types.h"

#include "driver/gpio.h"

/*
 * Keep GPIO defines ordered by GPIO_NUM_XX to avoid double assignments
 */

// extern const gpio_num_t ROTENC_GPIO_CLK;
// extern const gpio_num_t ROTENC_GPIO_DT;
// extern const gpio_num_t ROTENC_GPIO_SW;
extern const gpio_num_t I2C_P0_GPIO_SDA;
extern const gpio_num_t I2C_P0_GPIO_SCL;
extern const gpio_num_t OLED_GPIO_RESET;
extern const gpio_num_t WS2812_GPIO;

#endif
