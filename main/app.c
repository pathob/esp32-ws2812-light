#include "app.h"

static const char *TAG = "APP";

static WS2812_stripe_t stripe;
static const uint8_t stripe_length = 20;

static i2c_port_t i2c_port0 = I2C_NUM_0;

static WS2812_color_t warmwhite = { 255, 150, 70 };

void app_main()
{
    esp_err_t esp_err;
    
    
    WIFI_init(WIFI_MODE_STA, NULL);
    gpio_install_isr_service(0);

    // Init WS2812 stripe

    stripe.gpio_num = WS2812_GPIO;
    stripe.length = stripe_length;
    stripe.rmt_channel = RMT_CHANNEL_0;
    stripe.rmt_interrupt_num = 0;

    esp_err = WS2812_init(&stripe);
    if (!esp_err) {
        for (uint8_t x = 0; x <  stripe.length; x++) {
            WS2812_set_color(&stripe, x, &warmwhite);
        }
        WS2812_write(&stripe);
    }

    // Init I2C bus and sensors

    return;
    
    i2c_config_t i2c_port0_conf;
    i2c_port0_conf.mode = I2C_MODE_MASTER;
    i2c_port0_conf.sda_io_num = I2C_P0_GPIO_SDA;
    i2c_port0_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.scl_io_num = I2C_P0_GPIO_SCL;
    i2c_port0_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.master.clk_speed = 1600000;

    I2C_init(i2c_port0, &i2c_port0_conf);
}
