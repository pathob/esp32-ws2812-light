#ifndef __MAIN_ISR_GPIO_H__
#define __MAIN_ISR_GPIO_H__

#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_types.h"

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "periphery/delay.h"

#include "main/stripe.h"

void ISR_GPIO_init();

#endif
