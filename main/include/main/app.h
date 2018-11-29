#ifndef __MAIN_APP_H__
#define __MAIN_APP_H__

#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <math.h>

#include "esp_attr.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "esp_types.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "driver/rmt.h"

#include "connectivity/wifi.h"
#include "connectivity/ota.h"
#include "connectivity/sntp.h"

#include "periph/i2c.h"
#include "periph/delay.h"
#include "periph/ws2812.h"
#include "periph/display/bitmap.h"
#include "periph/display/ssd1306_i2c.h"
// #include "periph/display/keyboard_64x128.h"
#include "periph/font/lcd5x7.h"
#include "periph/sensor/bme280_i2c.h"
// #include "periph/sensor/rotary_encoder.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "main/defs.h"
#include "main/isr_gpio.h"
#include "main/mqtt.h"

#endif
