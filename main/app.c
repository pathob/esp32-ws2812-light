#include "app.h"

static const char *TAG = "APP";

static WS2812_stripe_t stripe;
static volatile uint8_t stripe_state = 0; 
static const uint8_t stripe_length = 56;

static i2c_port_t i2c_port0 = I2C_NUM_0;

static WS2812_color_t warmwhite = { 255, 150, 70 };

static QueueHandle_t gpio_intr_queue = NULL;
static unsigned long gpio_intr_last = 0;

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

    stripe_state = 1;
    ESP_LOGI(TAG, "LED on");
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

    stripe_state = 0;
    ESP_LOGI(TAG, "LED off");
}

static void toggle()
{
    stripe_state == 0 ? on() : off();
}

static void IRAM_ATTR gpio_isr(void* args)
{
    gpio_num_t gpio_num = (gpio_num_t) args;

    unsigned long us = micros();

    if (us - gpio_intr_last > 100000) {
        xQueueSendFromISR(gpio_intr_queue, &gpio_num, NULL);
        gpio_intr_last = us;
    }
}

static void IRAM_ATTR gpio_isr_task(void* pvParams)
{
    gpio_num_t gpio_num;
    BaseType_t x = pdFALSE;
    unsigned long us = 0;

    while(1) {
        xQueueReceive(gpio_intr_queue, &gpio_num, portMAX_DELAY);
        toggle();
    }
}

// webserver stuff

static void ledWebsocketConnect(
    Websock *ws);
static void ledWebsocketRecv(
    Websock *ws,
    char *data,
    int len,
    int flags);
static void ledWebsocketSend();

static HttpdFreertosInstance httpd_instance;
static uint8_t *httpd_buffer;
static Websock *websockets[4];
static const HttpdBuiltInUrl builtInUrls[] = {
    ROUTE_REDIRECT("/", "/index.html"),
    ROUTE_WS("/websocket/led", ledWebsocketConnect),
    ROUTE_FILESYSTEM(),
    ROUTE_END()
};

static void ledWebsocketConnect(
    Websock *ws)
{
	ws->recvCb = ledWebsocketRecv;

    for (uint8_t i = 0; i < 4; i++) {
        if (websockets[i] == NULL) {
            websockets[i] = ws;
            break;
        }
    }

    ledWebsocketSend();
}

static void ledWebsocketClose(
    Websock *ws)
{
    for (uint8_t i = 0; i < 4; i++) {
        if (websockets[i] == ws) {
            websockets[i] = NULL;
            break;
        }
    }
}

static void ledWebsocketRecv(
    Websock *ws,
    char *data,
    int len,
    int flags)
{
    if (len == 5 && strncmp(data, "false", 5) == 0) {
        if (stripe_state) {
            off();
        }
    } else if (len == 4 && strncmp(data, "true", 4) == 0) {
        if (!stripe_state) {
            on();
        }
    }
    ledWebsocketSend();
}

static void ledWebsocketSend()
{
    char send[6];

    if (stripe_state) {
        strcpy(send, "true");
    } else {
        strcpy(send, "false");
    }

    for (uint8_t i = 0; i < 4; i++) {
        if (websockets[i] != NULL) {
            cgiWebsocketSend(&httpd_instance.httpdInstance, websockets[i], send, strlen(send), WEBSOCK_FLAG_NONE);
        }
    }
}

void app_main()
{
    esp_err_t esp_err;
    
    gpio_install_isr_service(0);

    // Init WS2812 stripe

    stripe.gpio_num = WS2812_GPIO;
    stripe.length = stripe_length;
    stripe.rmt_channel = RMT_CHANNEL_0;
    stripe.rmt_interrupt_num = 0;

    esp_err = WS2812_init(&stripe);
    if (!esp_err) {
        on();
    }

    WIFI_init(WIFI_MODE_STA, NULL);

	espFsInit((void*)(webpages_espfs_start));

    uint8_t maxConnections = 16;
    httpd_buffer = malloc(sizeof(RtosConnType) * maxConnections);

    httpdFreertosInit(
        &httpd_instance,
        builtInUrls,
        80,
        httpd_buffer,
        maxConnections,
        HTTPD_FLAG_NONE);

    gpio_config_t gpio_conf;
    gpio_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    gpio_conf.pin_bit_mask = BIT(0);
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&gpio_conf);

    gpio_intr_queue = xQueueCreate(10, sizeof(gpio_num_t));
    xTaskCreate(gpio_isr_task, "gpio_isr_task", 4096, NULL, (1 | portPRIVILEGE_BIT), NULL);
    gpio_isr_handler_add(0, gpio_isr, (void *) 0);

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
