#include "stripe.h"

static char *TAG = "STRIPE";

static uint8_t _stripe_state;

void STRIPE_init() {

}

uint8_t STRIPE_state() {
    return _stripe_state;
}

void STRIPE_on() {
    ESP_LOGI(TAG, "On");
}

void STRIPE_off() {
    ESP_LOGI(TAG, "Off");
}
