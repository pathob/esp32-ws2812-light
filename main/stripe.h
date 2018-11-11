#ifndef __STRIPE_H__
#define __STRIPE_H__

#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_types.h"

void STRIPE_init();

uint8_t STRIPE_state();

void STRIPE_on();

void STRIPE_off();

#endif