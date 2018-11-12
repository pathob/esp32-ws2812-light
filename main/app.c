#include "app.h"

static const char *TAG = "APP";

static i2c_port_t i2c_port0 = I2C_NUM_0;

static QueueHandle_t gpio_intr_queue = NULL;
static unsigned long gpio_intr_last = 0;

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

    while(1) {
        xQueueReceive(gpio_intr_queue, &gpio_num, portMAX_DELAY);
        STRIPE_toggle();
    }
}

void app_main()
{
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

    ESP_LOGI(TAG, "Starting MQTT");
    xTaskCreate(&MQTT_task, "MQTT_task", 4096, NULL, 10, NULL);

    HTTPD_init();
    STRIPE_init();

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
