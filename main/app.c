#include "main/app.h"

static const char *TAG = "APP";

void app_main()
{
    gpio_install_isr_service(0);

    gpio_pad_select_gpio(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 1);

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

    HTTPD_init();

    STRIPE_init();

    ISR_GPIO_init();
}
