#ifndef MAIN_APP_H
#define MAIN_APP_H

#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/rmt.h"

#include "wifi.h"
#include "ota.h"
#include "ws2812.h"

#include "i2c.h"
#include "sensor/bme280_i2c.h"

#include "MQTTESP32.h"
#include "MQTTClient.h"

/*
 * Keep GPIO defines ordered by GPIO_NUM_XX to avoid double assignments
 */

#define I2C_P0_SCL_GPIO_NUM    GPIO_NUM_16    // pin 25
#define I2C_P0_SDA_GPIO_NUM    GPIO_NUM_5     // pin 34

#define WS2812_GPIO_NUM        GPIO_NUM_22    // pin 39

/*
 * Other defines
 */

#define MQTT_HOST      CONFIG_MQTT_HOST
#define MQTT_PORT      CONFIG_MQTT_PORT
#define MQTT_CLIENT_ID CONFIG_MQTT_CLIENT_ID
#define MQTT_USERNAME  CONFIG_MQTT_USERNAME
#define MQTT_PASSWORD  CONFIG_MQTT_PASSWORD
#define MQTT_TASK      CONFIG_MQTT_TASK

#endif
