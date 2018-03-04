#ifndef MAIN_APP_H
#define MAIN_APP_H

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

#include "wifi.h"
#include "ota.h"
#include "periph/i2c.h"
#include "periph/delay.h"
#include "periph/ws2812.h"
#include "periph/display/ssd1306_i2c.h"
#include "periph/display/keyboard_64x128.h"
#include "periph/font/lcd5x7.h"
#include "periph/sensor/bme280_i2c.h"
// #include "periph/sensor/rotary_encoder.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "libesphttpd/esp.h"
#include "libesphttpd/httpd.h"
#include "libesphttpd/httpd-freertos.h"
#include "libesphttpd/httpdespfs.h"
#include "libesphttpd/cgiwifi.h"
#include "libesphttpd/cgiflash.h"
#include "libesphttpd/auth.h"
#include "libesphttpd/espfs.h"
#include "libesphttpd/captdns.h"
#include "libesphttpd/webpages-espfs.h"
#include "libesphttpd/cgiwebsocket.h"
#include "libesphttpd/route.h"

#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <math.h>

#include "MQTTESP32.h"
#include "MQTTClient.h"

/*
 * Keep GPIO defines ordered by GPIO_NUM_XX to avoid double assignments
 */

const gpio_num_t ROTENC_GPIO_CLK   = GPIO_NUM_16;   // pin
const gpio_num_t ROTENC_GPIO_DT    = GPIO_NUM_17;   // pin
const gpio_num_t ROTENC_GPIO_SW    = GPIO_NUM_18;   // pin
const gpio_num_t I2C_P0_GPIO_SDA   = GPIO_NUM_26;
const gpio_num_t I2C_P0_GPIO_SCL   = GPIO_NUM_27;
const gpio_num_t WS2812_GPIO       = GPIO_NUM_33;

/*
 * Other defines
 */

#define MQTT_HOST      CONFIG_MQTT_HOST
#define MQTT_PORT      CONFIG_MQTT_PORT
#define MQTT_CLIENT_ID CONFIG_MQTT_CLIENT_ID
#define MQTT_USERNAME  CONFIG_MQTT_USERNAME
#define MQTT_PASSWORD  CONFIG_MQTT_PASSWORD
#define MQTT_USE_TASK  CONFIG_MQTT_USE_TASK

#endif
