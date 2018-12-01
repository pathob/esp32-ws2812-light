#ifndef __MAIN_DISPLAY_H__
#define __MAIN_DISPLAY_H__

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

#include "connectivity/connectivity.h"
#include "connectivity/ota.h"
#include "connectivity/sntp.h"
#include "connectivity/wifi.h"

#include "periphery/i2c.h"
#include "periphery/delay.h"
#include "periphery/ws2812.h"
#include "periphery/display/bitmap.h"
#include "periphery/display/ssd1306_i2c.h"
// #include "periphery/display/keyboard_64x128.h"
#include "periphery/font/lcd5x7.h"
#include "periphery/sensor/bme280_i2c.h"
// #include "periphery/sensor/rotary_encoder.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "main/mqtt.h"
#include "main/defs.h"


void DISPLAY_task(
    void *pvParameters);

#endif
