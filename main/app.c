#include "main/app.h"

static const char *TAG = "APP";

void app_main()
{
    gpio_install_isr_service(0);

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);

    WIFI_init(WIFI_MODE_STA , NULL);

    OTA_init();

    SNTP_init();

    MQTT_callback_handler_t mqtt_callback_handler = {
        .init_handler = MQTT_init_handler,
        .connected_handler = MQTT_connected_handler,
        .data_handler = MQTT_data_handler,
    };

    MQTT_init(&mqtt_callback_handler);

    i2c_config_t i2c_port0_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_P0_GPIO_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_P0_GPIO_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 1600000,
    };

    I2C_init(I2C_PORT, &i2c_port0_conf);

    // ESP_ERROR_CHECK( SSD1306_init_with_reset(i2c_port0, SSD1306_ADDR_LOW, OLED_GPIO_RESET) );
    // ESP_ERROR_CHECK( BME280_init(i2c_port0, BME280_ADDR_LOW) );
    // xTaskCreate(&SSD1306_task, "SSD1306_task", 2048, NULL, 10, NULL);

    HTTPD_init();

    STRIPE_init();

    ISR_GPIO_init();
}
