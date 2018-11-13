#include "app.h"

static const char *TAG = "APP";

void app_main()
{
    gpio_install_isr_service(0);

    // Init I2C bus and sensors

    // ESP_ERROR_CHECK( SSD1306_init_with_reset(i2c_port0, SSD1306_ADDR_LOW, OLED_GPIO_RESET) );
    // ESP_ERROR_CHECK( BME280_init(i2c_port0, BME280_ADDR_LOW) );
    // xTaskCreate(&SSD1306_task, "SSD1306_task", 2048, NULL, 10, NULL);

    WIFI_init(WIFI_MODE_STA , NULL);

    ESP_LOGI(TAG, "Starting MQTT");
    xTaskCreate(&MQTT_task, "MQTT_task", 4096, NULL, 10, NULL);

    i2c_config_t i2c_port0_conf;
    i2c_port0_conf.mode = I2C_MODE_MASTER;
    i2c_port0_conf.sda_io_num = I2C_P0_GPIO_SDA;
    i2c_port0_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.scl_io_num = I2C_P0_GPIO_SCL;
    i2c_port0_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.master.clk_speed = 1600000;

    I2C_init(I2C_PORT, &i2c_port0_conf);

    HTTPD_init();
    STRIPE_init();
    ISR_GPIO_init();

    return;
}
