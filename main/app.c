#include "app.h"

static const char *TAG = "APP";

static WS2812_stripe_t stripe;
static const uint8_t stripe_length = 56;

static i2c_port_t i2c_port0 = I2C_NUM_0;

static WS2812_color_t warmwhite = { 255, 150, 70 };

static void on()
{
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
}

static void off()
{
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
}

static void ledWebsocketRecv(
    Websock *ws,
    char *data,
    int len,
    int flags)
{
    if (len == 5 && strncmp(data, "false", 5) == 0) {
        off();
        ESP_LOGI(TAG, "LED off");
    } else if (len == 4 && strncmp(data, "true", 4) == 0) {
        on();
        ESP_LOGI(TAG, "LED on");
    }
}

static void ledWebsocketConnect(
    Websock *ws)
{
	ws->recvCb = ledWebsocketRecv;
}

static HttpdFreertosInstance httpd_instance;

static uint8_t *httpd_buffer;

static const HttpdBuiltInUrl builtInUrls[] = {
    ROUTE_REDIRECT("/", "/index.html"),
    ROUTE_WS("/websocket/led", ledWebsocketConnect),
    ROUTE_FILESYSTEM(),
    ROUTE_END()
};

void app_main()
{
    esp_err_t esp_err;
    
    WIFI_init(WIFI_MODE_AP, NULL);

	espFsInit((void*)(webpages_espfs_start));

    uint8_t maxConnections = 8;
    httpd_buffer = malloc(sizeof(RtosConnType) * maxConnections);

    httpdFreertosInit(
        &httpd_instance,
        builtInUrls,
        80,
        httpd_buffer,
        maxConnections,
        HTTPD_FLAG_NONE);

    // Init WS2812 stripe

    stripe.gpio_num = WS2812_GPIO;
    stripe.length = stripe_length;
    stripe.rmt_channel = RMT_CHANNEL_0;
    stripe.rmt_interrupt_num = 0;

    esp_err = WS2812_init(&stripe);
    if (!esp_err) {
        on();
    }

    return;

    // Init I2C bus and sensors
    
    i2c_config_t i2c_port0_conf;
    i2c_port0_conf.mode = I2C_MODE_MASTER;
    i2c_port0_conf.sda_io_num = I2C_P0_GPIO_SDA;
    i2c_port0_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.scl_io_num = I2C_P0_GPIO_SCL;
    i2c_port0_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.master.clk_speed = 1600000;

    I2C_init(i2c_port0, &i2c_port0_conf);
}
