#ifndef __MAIN_DEFS_H__
#define __MAIN_DEFS_H__

#include "esp_types.h"

#include "driver/gpio.h"
#include "driver/i2c.h"

/*
 * Keep GPIO defines ordered by GPIO_NUM_XX to avoid double assignments
 */

// extern const gpio_num_t ROTENC_GPIO_CLK;
// extern const gpio_num_t ROTENC_GPIO_DT;
// extern const gpio_num_t ROTENC_GPIO_SW;
extern const gpio_num_t LED_GPIO;
extern const gpio_num_t I2C_P0_GPIO_SDA;
extern const gpio_num_t I2C_P0_GPIO_SCL;
extern const gpio_num_t OLED_GPIO_RESET;
extern const gpio_num_t WS2812_GPIO;

extern const i2c_port_t I2C_PORT;

#endif
