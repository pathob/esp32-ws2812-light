#include "app.h"

static const char *TAG = "APP";

static WS2812_stripe_t stripe;
static const uint8_t stripe_length = 255;

void WIFI_connected()
{
    printf("Wifi connected\n");
}

void WIFI_disconnected()
{
    printf("Wifi disconnected\n");
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

    /*
    if (MQTT_TASK) {
        if ((rc = MQTTStartTask(&client)) != pdPASS)
            printf("Return code from start tasks is %d\n", rc);
    }
    */

    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = MQTT_CLIENT_ID;
    connectData.username.cstring = MQTT_USERNAME;
    connectData.password.cstring = MQTT_PASSWORD;

    if ((rc = MQTTConnect(&client, &connectData)) != 0)
        printf("Return code from MQTT connect is %d\n", rc);
    else
        printf("MQTT Connected\n");

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

        if (MQTT_TASK) {
            if ((rc = MQTTYield(&client, 10000)) != 0)
                printf("Return code from yield is %d\n", rc);
        }

        TickType_t xDelay = 500 / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
    }
}

void WS2812_task(void *pvParameters) {
    uint8_t add = 0;
    uint8_t sub = 2;

    uint8_t step = 4;

    while (1) {
        WS2812_color_t last;

        for (uint16_t i = stripe_length; i != 0; i--) {
            WS2812_get_color(&stripe, i-1, &last);
            WS2812_set_color(&stripe, i, &last);
        }

        uint8_t new[3] = { last.r, last.g, last.b };

        // if uint8_t running over
        if ((new[add] + step) % 255 < new[add]) {
            ESP_LOGI(TAG, "INC SUB UND ADD");
            sub = add;
            add = (add + 1) % 3;
        }

        new[add] = new[add] + step;

        // if uint8_t not running over
        if (new[sub] >= step) {
            new[sub] = new[sub] - step;
        }

        // ESP_LOGI(TAG, "New color: %d %d %d", new[0], new[1], new[2]);

        WS2812_color_t color = { new[0], new[1], new[2] };
        WS2812_set_color(&stripe, 0, &color);

        WS2812_write(&stripe);
        // vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void I2C_task(void *pvParameters)
{

}

void app_main()
{
    esp_err_t esp_err;

    nvs_flash_init();

    // WIFI_init(NULL);

    /*
     * Init WS2812 stripe
     */

    /*
    stripe.gpio_num = GPIO_NUM_22;
    stripe.length = stripe_length;
    stripe.rmt_channel = RMT_CHANNEL_0;
    stripe.rmt_interrupt_num = 0;

    esp_err = WS2812_init(&stripe);
    if (!esp_err) {
        ESP_LOGI(TAG, "WS2812 init done");
        xTaskCreate(&WS2812_task, "WS2812_task", 2048, NULL, 10, NULL);
    }
    */

    /*
     * Init I2C bus and sensors
     */

    i2c_port_t i2c_port0 = I2C_NUM_0;

    i2c_config_t i2c_port0_conf;
    i2c_port0_conf.mode = I2C_MODE_MASTER;
    i2c_port0_conf.sda_io_num = I2C_P0_SDA_GPIO_NUM;
    i2c_port0_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.scl_io_num = I2C_P0_SCL_GPIO_NUM;
    i2c_port0_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.master.clk_speed = 100000;

    I2C_init(i2c_port0, &i2c_port0_conf);
    BME280_init(i2c_port0, BME280_ADDR_HIGH);

    // TODO: How to decide if update should be started?
    // xTaskCreate(&OTA_task, "OTA_task", 2048, NULL, 10, NULL);

    /*
    esp_bt_controller_init();

    if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
        return;
    }

    ESP_LOGI("TAG", "TEST");

    xTaskCreate(&bleAdvtTask, "bleAdvtTask", 2048, (void* ) 0, 10, NULL);
//    xTaskCreate(&wifiTask,    "wifiTask",    2048, (void* ) 1, 10, NULL);
     */

    // don't call vTaskStartScheduler() because
    // the scheduler is already running per default
}
