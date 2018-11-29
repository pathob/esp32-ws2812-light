#include "main/stripe.h"

static char *TAG = "STRIPE";

static WS2812_stripe_t _stripe;
static volatile uint8_t _stripe_state = 0;
static const uint8_t _stripe_length = 56;

static const WS2812_color_t warmwhite = { 255, 150, 70 };
static const WS2812_color_t black = { 0, 0, 0 };

void STRIPE_init()
{
    // Init WS2812 stripe

    _stripe.gpio_num = WS2812_GPIO;
    _stripe.length = _stripe_length;
    _stripe.rmt_channel = RMT_CHANNEL_0;
    _stripe.rmt_interrupt_num = 0;

    if (WS2812_init(&_stripe) == ESP_OK) {
        STRIPE_on();
    }
}

uint8_t STRIPE_state()
{
    return _stripe_state;
}

void STRIPE_on()
{
    uint8_t old_state = _stripe_state;
    _stripe_state = 1;

    ledWebsocketBroadcast();
    MQTT_topic_led_broadcast();

    if (!old_state) {
        ESP_LOGI(TAG, "Turning on");

        for (uint8_t x = 0; x <  _stripe.length / 2; x++) {
            WS2812_set_color(&_stripe, x, &warmwhite);
            WS2812_set_color(&_stripe, _stripe_length-x-1, &warmwhite);
            WS2812_write(&_stripe);
            delay_ms(10);
        }
    }

    /*
    uint8_t steps = 18;
    uint8_t offset = 2;
    uint64_t delay = pow(2, steps);
    
    for (uint8_t i = offset; i < steps; i++) {
        uint8_t r = warmwhite.r * (i / (float) steps);
        uint8_t g = warmwhite.g * (i / (float) steps);
        uint8_t b = warmwhite.b * (i / (float) steps);

        WS2812_color_t color = { r, g, b };

        for (uint8_t x = 0; x <  stripe.length; x++) {
            WS2812_set_color(&stripe, x, &color);
        }

        WS2812_write(&stripe);

        delay_us(delay >> i);
    }
    */
}

void STRIPE_off()
{
    uint8_t old_state = _stripe_state;
    _stripe_state = 0;

    ledWebsocketBroadcast();
    MQTT_topic_led_broadcast();

    if (old_state) {
        ESP_LOGI(TAG, "Turning off");

        for (uint8_t x = 0; x <  _stripe.length / 2; x++) {
            WS2812_set_color(&_stripe, x, &black);
            WS2812_set_color(&_stripe, _stripe.length-x-1, &black);
            WS2812_write(&_stripe);
            delay_ms(10);
        }
    }

    /*
    uint8_t steps = 18;
    uint8_t offset = 2;
    uint64_t delay = pow(2, steps);

    for (uint8_t i = offset; i < steps; i++) {
        uint8_t r = warmwhite.r * ((steps-(i+1)) / (float) steps);
        uint8_t g = warmwhite.g * ((steps-(i+1)) / (float) steps);
        uint8_t b = warmwhite.b * ((steps-(i+1)) / (float) steps);

        WS2812_color_t color = { r, g, b };

        for (uint8_t x = 0; x <  stripe.length; x++) {
            WS2812_set_color(&stripe, x, &color);
        }

        WS2812_write(&stripe);

        delay_us(delay >> (i-offset));
    }
    */
}

void STRIPE_toggle()
{
    STRIPE_state() == 0 ? STRIPE_on() : STRIPE_off(); // Toggle
}
