#include "main/stripe.h"

static char *TAG = "STRIPE";

static WS2812_stripe_t _stripe;
// This is the actual color that has been set for the stripe
static volatile WS2812_color_t _stripe_color = { 0, 0, 0 };
static xTaskHandle _stripe_task = NULL;

// These are the values that are received by MQTT or websocket
// but probably are not reached when the task is aborted
static volatile uint8_t _stripe_state = 0;
static volatile uint8_t _stripe_brightness = 255;
static volatile WS2812_color_t _stripe_color_rgb = { 0, 0, 0 };
static volatile uint16_t _stripe_color_mired = 588;
static volatile uint32_t _stripe_transition_ms = 1000;

// These values should be read from NVS instead
static const uint8_t _stripe_length = 166;
static const uint8_t _stripe_interval = 1;

static void STRIPE_set_task(
    void *pvParameters);

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
    WS2812_color_t *color_rgb,
    uint32_t transition_ms)
{
    if (state) {
        _stripe_state = *state ? 1 : 0;
    }

    if (brightness) {
        _stripe_brightness = *brightness;
    }

    if (color_rgb) {
        _stripe_color_rgb = *color_rgb;
    }

    _stripe_transition_ms = transition_ms;

    ESP_LOGI(TAG, "Set to state %d, brightness %d, color %d,%d,%d",
        _stripe_state, _stripe_brightness, _stripe_color_rgb.r, _stripe_color_rgb.g, _stripe_color_rgb.b);

    // The answer should be sent immediatelly, no matter what color has actually been set
    STRIPE_mqtt_publish_status();
    // STRIPE_websocket_broadcast_status();

    // Stop old task when it is running
    if (_stripe_task) {
        ESP_LOGI(TAG, "Task is running, delete now");
        vTaskDelete(_stripe_task);
        _stripe_task = NULL;
    }

    // Start task
    xTaskCreate(STRIPE_set_task, "STRIPE_set_task", 4096, NULL, 10, &_stripe_task);
}

static void STRIPE_set_task(
    void *pvParameters)
{
    // The color to start with is taken from the actual last set color
    uint8_t old_r = _stripe_color.r;
    uint8_t old_g = _stripe_color.g;
    uint8_t old_b = _stripe_color.b;

    // The color to transition to is calculated from new state, brightness and rgb color
    float factor = _stripe_state * (_stripe_brightness / 255.0f);
    uint8_t new_r = (uint8_t) (factor * (float) _stripe_color_rgb.r);
    uint8_t new_g = (uint8_t) (factor * (float) _stripe_color_rgb.g);
    uint8_t new_b = (uint8_t) (factor * (float) _stripe_color_rgb.b);

    const uint8_t steps = 50;
    const uint16_t delay = _stripe_transition_ms / steps;
    float step_r = (new_r - old_r) / (float) steps;
    float step_g = (new_g - old_g) / (float) steps;
    float step_b = (new_b - old_b) / (float) steps;

    for (uint8_t i = 0; i < steps; i++) {
        WS2812_color_t c = {
            old_r + (uint8_t) ((i + 1) * step_r),
            old_g + (uint8_t) ((i + 1) * step_g),
            old_b + (uint8_t) ((i + 1) * step_b)
        };

        ESP_LOGI(TAG, "Set computed color %d,%d,%d", c.r, c.g, c.b);

        for (uint8_t x = 0; x <  _stripe.length; x += _stripe_interval) {
            WS2812_set_color(&_stripe, x, &c);
        }

        WS2812_write(&_stripe);
        _stripe_color = c;
        delay_ms(delay);
    }

    _stripe_task = NULL;
    vTaskDelete(NULL);
}

void STRIPE_set_json(
    char *data)
{
    uint8_t *stripe_state = NULL;
    uint8_t *stripe_brightness = NULL;
    WS2812_color_t *stripe_color = NULL;
    uint32_t stripe_transition_ms = 1000;

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

    const cJSON *transition = cJSON_GetObjectItemCaseSensitive(json, "transition");
    if (cJSON_IsNumber(transition)) {
        ESP_LOGD(TAG, "Found transition in JSON");

        stripe_transition_ms = transition->valueint * 1000;
    }

    const cJSON *color_temp = cJSON_GetObjectItemCaseSensitive(json, "color_temp");
    if (cJSON_IsNumber(color_temp)) {
        ESP_LOGD(TAG, "Found color_temp in JSON");
        WS2812_color_t c;

        _stripe_color_mired = color_temp->valueint;
        LIGHT_rgb_from_mired(_stripe_color_mired, (LIGHT_rgb_t*) &c);
        stripe_color = &c;
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

    STRIPE_set(stripe_state, stripe_brightness, stripe_color, stripe_transition_ms);
}

uint8_t STRIPE_get_state()
{
    return _stripe_state;
}

void STRIPE_toggle()
{
    uint8_t state = !_stripe_state;
    STRIPE_set(&state, NULL, NULL, 1000);
}

static void STRIPE_websocket_broadcast_status()
{
    HTTPD_websocket_led_broadcast(_stripe_state ? "1" : "0", 1);
}

static void STRIPE_mqtt_publish_status()
{
    char mqtt_data[128] = { 0 };
    uint8_t mqtt_data_len;

    mqtt_data_len = sprintf(mqtt_data, "{\"state\":\"%s\",\"brightness\":%u,\"color\":{\"r\":%u,\"g\":%u,\"b\":%u},\"color_temp\":%u}",
        _stripe_state ? "ON" : "OFF",
        _stripe_brightness,
        _stripe_color_rgb.r,
        _stripe_color_rgb.g,
        _stripe_color_rgb.b,
        _stripe_color_mired
    );

    MQTT_publish_rgb_status(mqtt_data, mqtt_data_len);
}

