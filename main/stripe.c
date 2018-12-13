#include "main/stripe.h"

static char *TAG = "STRIPE";

static WS2812_stripe_t _stripe;
static volatile uint8_t _stripe_state = 0;
static volatile uint8_t _stripe_brightness = 255;
static volatile WS2812_color_t _stripe_color = { 0 };
static const uint8_t _stripe_length = 56;

// static const WS2812_color_t warmwhite = { 255, 150, 70 };

static void STRIPE_websocket_broadcast_status();

static void STRIPE_mqtt_publish_status();

void STRIPE_init()
{
    // Init WS2812 stripe

    _stripe.gpio_num = WS2812_GPIO;
    _stripe.length = _stripe_length;
    _stripe.rmt_channel = RMT_CHANNEL_0;
    _stripe.rmt_interrupt_num = 0;

    ESP_ERROR_CHECK(WS2812_init(&_stripe));
}

void STRIPE_set(
    uint8_t *state,
    uint8_t *brightness,
    WS2812_color_t *color)
{
    if (state) {
        _stripe_state = *state ? 1 : 0;
    }

    if (brightness) {
        _stripe_brightness = *brightness;
    }

    if (color) {
        _stripe_color = *color;
    }

    ESP_LOGI(TAG, "Set to state %d, brightness %d, color %d,%d,%d",
        _stripe_state, _stripe_brightness, _stripe_color.r, _stripe_color.g, _stripe_color.b
    );

    STRIPE_websocket_broadcast_status();
    STRIPE_mqtt_publish_status();

    float factor = _stripe_state * (_stripe_brightness / 255.0);
    uint8_t r = (uint8_t) factor * _stripe_color.r;
    uint8_t g = (uint8_t) factor * _stripe_color.g;
    uint8_t b = (uint8_t) factor * _stripe_color.b;

    WS2812_color_t c = { r, g, b };

    for (uint8_t x = 0; x <  _stripe.length / 2; x++) {
        WS2812_set_color(&_stripe, x, &c);
        WS2812_set_color(&_stripe, _stripe_length-x-1, &c);
        WS2812_write(&_stripe);
        delay_ms(10);
    }
}

void STRIPE_set_json(
    char *data)
{
    uint8_t *stripe_state = NULL;
    uint8_t *stripe_brightness = NULL;
    WS2812_color_t *stripe_color = NULL;

    cJSON *json = cJSON_Parse(data);

    if (json == NULL) {
        ESP_LOGE(TAG, "Cannot parse JSON data");
        return;
    }

    const cJSON *state = cJSON_GetObjectItemCaseSensitive(json, "state");
    if (cJSON_IsString(state) && (state->valuestring != NULL)) {
        ESP_LOGD(TAG, "Found state in JSON");
        uint8_t s = 0;
        if (strcmp(state->valuestring, "ON") == 0) {
            s = 1;
        }
        stripe_state = &s;
    }

    const cJSON *brightness = cJSON_GetObjectItemCaseSensitive(json, "brightness");
    if (cJSON_IsNumber(brightness)) {
        ESP_LOGD(TAG, "Found brightness in JSON");
        stripe_brightness = (uint8_t *) &brightness->valueint; 
    }

    const cJSON *color = cJSON_GetObjectItemCaseSensitive(json, "color");
    if (cJSON_IsObject(color)) {
        ESP_LOGD(TAG, "Found color in JSON");
        WS2812_color_t c;

        const cJSON *r = cJSON_GetObjectItemCaseSensitive(color, "r");
        if (cJSON_IsNumber(r)) {
            c.r = (uint8_t) r->valueint;
        }

        const cJSON *g = cJSON_GetObjectItemCaseSensitive(color, "g");
        if (cJSON_IsNumber(g)) {
            c.g = (uint8_t) g->valueint;
        }

        const cJSON *b = cJSON_GetObjectItemCaseSensitive(color, "b");
        if (cJSON_IsNumber(b)) {
            c.b = (uint8_t) b->valueint;
        }

        stripe_color = &c;
    }

    STRIPE_set(stripe_state, stripe_brightness, stripe_color);
}

uint8_t STRIPE_get_state()
{
    return _stripe_state;
}

void STRIPE_toggle()
{
    uint8_t state = !_stripe_state;
    STRIPE_set(&state, NULL, NULL);
}

static void STRIPE_websocket_broadcast_status()
{
    HTTPD_websocket_led_broadcast(_stripe_state ? "1" : "0", 1);
}

static void STRIPE_mqtt_publish_status()
{
    char mqtt_data[128] = { 0 };
    uint8_t mqtt_data_len;

    mqtt_data_len = sprintf(mqtt_data, "{\"state\":\"%s\",\"brightness\":%u,\"color\":{\"r\":%u,\"g\":%u,\"b\":%u}}",
        _stripe_state ? "ON" : "OFF",
        _stripe_brightness,
        _stripe_color.r,
        _stripe_color.g,
        _stripe_color.b
    );

    MQTT_publish_rgb_status(mqtt_data, mqtt_data_len);
}