#ifndef __MAIN_MQTT_H__
#define __MAIN_MQTT_H__

#include "esp_attr.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "esp_types.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "mqtt_client.h"
#include "mqtt_config.h"

#include "main/stripe.h"

#include "sdkconfig.h"

void MQTT_task(
    void *pvParameters);

void MQTT_topic_led_broadcast();

#endif