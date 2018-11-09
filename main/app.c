#include "app.h"

static const char *TAG = "APP";

static WS2812_stripe_t stripe;
static volatile uint8_t stripe_state = 0; 
static const uint8_t stripe_length = 24;

static i2c_port_t i2c_port0 = I2C_NUM_0;

static WS2812_color_t warmwhite = { 255, 150, 70 };

static QueueHandle_t gpio_intr_queue = NULL;
static unsigned long gpio_intr_last = 0;

// webserver stuff

static void ledWebsocketConnect(
    Websock *ws);
static void ledWebsocketRecv(
    Websock *ws,
    char *data,
    int len,
    int flags);
static void ledWebsocketBroadcast();

static HttpdFreertosInstance httpd_instance;
static uint8_t *httpd_buffer;
static const HttpdBuiltInUrl builtInUrls[] = {
    ROUTE_REDIRECT("/", "/index.html"),
    ROUTE_REDIRECT("/apple-touch-icon.png", "/launcher-icon-48.png"),
    ROUTE_WS("/websocket/led", ledWebsocketConnect),
    ROUTE_FILESYSTEM(),
    ROUTE_END()
};

void SSD1306_task(void *pvParameters)
{
    EventBits_t event_bits = 0;
    ESP_LOGI(TAG, "Starting SSD1306 task");

    SSD1306_set_bitmap(espressif, 124, 24, 2, 20);
    SSD1306_display();
    vTaskDelay(333 / portTICK_PERIOD_MS);

    SSD1306_set_bitmap(wifi, 36, 24, 46, 20);
    SSD1306_display();
    vTaskDelay(333 / portTICK_PERIOD_MS);

    SSD1306_set_bitmap(bluetooth, 90, 22, 19, 21);
    SSD1306_display();
    vTaskDelay(333 / portTICK_PERIOD_MS);

    event_bits = xEventGroupGetBits(WIFI_event_group);

    SSD1306_set_text_6x8(FONT_lcd5x7, "Connecting to...", 4, 23);
    SSD1306_set_text_6x8(FONT_lcd5x7, WIFI_STA_SSID, 4, 33);
    SSD1306_display();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    xEventGroupWaitBits(WIFI_event_group, WIFI_STA_CONNECTED_BIT, false, true, portMAX_DELAY);

    SSD1306_set_text_6x8(FONT_lcd5x7, "Fetching time...", 4, 28);
    SSD1306_display();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    xEventGroupWaitBits(WIFI_event_group, SNTP_TIME_SET_BIT, false, true, portMAX_DELAY);

    time_t t;
    timeinfo_t timeinfo = { 0 };

    char buffer[50];
    char strftime_buf[6];

    uint8_t buffer_bitmap_8x8[64];

    char buffer_ip[20] = { 0 };

    if (WIFI_sta_is_connected()) {
        ip4_addr_t ip4_addr = WIFI_sta_ip4_addr();

        sprintf(buffer_ip, "IP: %d.%d.%d.%d"
            , (ip4_addr.addr >> 0)  & 0xFF
            , (ip4_addr.addr >> 8)  & 0xFF
            , (ip4_addr.addr >> 16) & 0xFF
            , (ip4_addr.addr >> 24) & 0xFF
        );
    }
    
    while (1) {
        time(&t);
        localtime_r(&t, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%R", &timeinfo);

        SSD1306_set_text_6x8(FONT_lcd5x7, strftime_buf, 95, 4);
        SSD1306_set_bitmap(bluetooth_icon_8x8, 8, 8, 85, 3);

        WIFI_sta_rssi_bitmap_8x8(&buffer_bitmap_8x8);
        SSD1306_set_bitmap(buffer_bitmap_8x8, 8, 8, 75, 3);

        SSD1306_set_bitmap(wifi_icon_8x8, 8, 8, 65, 3);
        SSD1306_set_text_6x8(FONT_lcd5x7, buffer_ip, 4, 24  );
        SSD1306_display();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

// implementation

static void on()
{
    if (stripe_state) {
        return;
    }

    ESP_LOGI(TAG, "LED on");
    stripe_state = 1;

    uint8_t steps = 18;
    uint8_t offset = 2;
    uint64_t delay = pow(2, steps);

    ledWebsocketBroadcast();

    for (uint8_t x = 0; x <  stripe.length; x++) {
        WS2812_set_color(&stripe, x, &warmwhite);
        WS2812_write(&stripe);
        delay_ms(10);
    }

    /*
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

static void off()
{
    if (!stripe_state) {
        return;
    }

    ESP_LOGI(TAG, "LED off");
    stripe_state = 0;

    uint8_t steps = 18;
    uint8_t offset = 2;
    uint64_t delay = pow(2, steps);

    ledWebsocketBroadcast();

    WS2812_color_t black = { 0, 0, 255 };

    for (uint8_t x = 0; x <  stripe.length; x++) {
        WS2812_set_color(&stripe, x, &black);
        WS2812_write(&stripe);
        delay_ms(10);
    }

    /*
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
        // Toggle:
        stripe_state == 0 ? on() : off();
    }
}

static void ledWebsocketConnect(
    Websock *ws)
{
	ws->recvCb = ledWebsocketRecv;
    
    char send[1];
    sprintf(send, "%d", stripe_state);
    cgiWebsocketSend(&httpd_instance.httpdInstance, ws, send, 1, WEBSOCK_FLAG_NONE);
}

static void ledWebsocketRecv(
    Websock *ws,
    char *data,
    int len,
    int flags)
{
    if (len == 1) {
        strncmp(data, "0", 1) == 0 ? off() : on();
    }
}

static void ledWebsocketBroadcast()
{
    char send[1];
    sprintf(send, "%d", stripe_state);
    cgiWebsockBroadcast(&httpd_instance.httpdInstance, "/websocket/led", send, 1, WEBSOCK_FLAG_NONE);
}

void messageArrived(MessageData* data)
{
    printf("Message arrived on topic %.*s: %.*s\n",
            data->topicName->lenstring.len,
            (char *) data->topicName->lenstring.data,
            data->message->payloadlen,
            (char *) data->message->payload);
}

void MQTT_task(void *pvParameters)
{
    MQTTClient client;
    Network network;
    unsigned char sendbuf[80], readbuf[80];
    int rc = 0;
    int count = 0;

    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    NetworkInit(&network);
    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    if ((rc = NetworkConnect(&network, MQTT_HOST, MQTT_PORT)) != 0)
        printf("Return code from network connect is %d\n", rc);

    if (CONFIG_MQTT_USE_TASK) {
        if ((rc = MQTTStartTask(&client)) != pdPASS) {
            printf("Return code from start tasks is %d\n", rc);
        }
    }

    connectData.MQTTVersion      = CONFIG_MQTT_VERSION;
    connectData.clientID.cstring = CONFIG_MQTT_CLIENT_ID;
    connectData.username.cstring = CONFIG_MQTT_USERNAME;
    connectData.password.cstring = CONFIG_MQTT_PASSWORD;

    if ((rc = MQTTConnect(&client, &connectData)) != 0) {
        printf("Return code from MQTT connect is %d\n", rc);
    }
    else {
        printf("MQTT Connected\n");
    }

    if ((rc = MQTTSubscribe(&client, "FreeRTOS/sample/#", 2, messageArrived)) != 0)
        printf("Return code from MQTT subscribe is %d\n", rc);

    while (++count)
    {
        MQTTMessage message;
        char payload[30];

        message.qos = 1;
        message.retained = 0;
        message.payload = payload;
        sprintf(payload, "message number %d", count);
        message.payloadlen = strlen(payload);

        if ((rc = MQTTPublish(&client, "FreeRTOS/sample/a", &message)) != 0)
            printf("Return code from MQTT publish is %d\n", rc);

        if (CONFIG_MQTT_USE_TASK) {
            if ((rc = MQTTYield(&client, 10000)) != 0)
                printf("Return code from yield is %d\n", rc);
        }

        TickType_t xDelay = 60000 / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
    }
}

void app_main()
{
    esp_err_t esp_err;
    gpio_install_isr_service(0);

    // Init I2C bus and sensors

    i2c_config_t i2c_port0_conf;
    i2c_port0_conf.mode = I2C_MODE_MASTER;
    i2c_port0_conf.sda_io_num = I2C_P0_GPIO_SDA;
    i2c_port0_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.scl_io_num = I2C_P0_GPIO_SCL;
    i2c_port0_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.master.clk_speed = 1600000;

    I2C_init(i2c_port0, &i2c_port0_conf);

    // ESP_ERROR_CHECK( SSD1306_init_with_reset(i2c_port0, SSD1306_ADDR_LOW, OLED_GPIO_RESET) );
    // ESP_ERROR_CHECK( BME280_init(i2c_port0, BME280_ADDR_LOW) );
    // xTaskCreate(&SSD1306_task, "SSD1306_task", 2048, NULL, 10, NULL);

    WIFI_init(WIFI_MODE_STA , NULL);

    // Init WS2812 stripe

    stripe.gpio_num = WS2812_GPIO;
    stripe.length = stripe_length;
    stripe.rmt_channel = RMT_CHANNEL_0;
    stripe.rmt_interrupt_num = 0;

    espFsInit((void*)(webpages_espfs_start));

    uint8_t maxConnections = 24;
    httpd_buffer = malloc(sizeof(RtosConnType) * maxConnections);

    httpdFreertosInit(
        &httpd_instance,
        builtInUrls,
        80,
        httpd_buffer,
        maxConnections,
        HTTPD_FLAG_NONE);

    httpdFreertosStart(&httpd_instance);

    uint8_t mac[6];
    char mac_str[12];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    esp_log_buffer_hex(TAG, mac, sizeof(mac));

    sprintf(mac_str, "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    


    xTaskCreate(&MQTT_task, "MQTT_task", 4096, NULL, 10, NULL);

    esp_err = WS2812_init(&stripe);
    if (!esp_err) {
        on();
    }

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
}
