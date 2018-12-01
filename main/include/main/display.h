#ifndef __MAIN_DISPLAY_H__
#define __MAIN_DISPLAY_H__

#include "connectivity/sntp.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "periphery/display/bitmap.h"
#include "periphery/display/ssd1306_i2c.h"

void DISPLAY_task(
    void *pvParameters);

#endif