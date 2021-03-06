#include "main/display.h"

static const char *TAG = "DISPLAY";

static void WIFI_sta_rssi_bitmap_8x8(
    uint8_t *bitmap);

void DISPLAY_task(
    void *pvParameters)
{
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

    SSD1306_set_text_6x8(FONT_lcd5x7, "Connecting to...", 4, 23);
    SSD1306_set_text_6x8(FONT_lcd5x7, CONFIG_WIFI_STA_SSID, 4, 33);
    SSD1306_display();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    WIFI_sta_connectivity_wait();

    SSD1306_set_text_6x8(FONT_lcd5x7, "Fetching time...", 4, 28);
    SSD1306_display();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    SNTP_connectivity_wait();

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

        WIFI_sta_rssi_bitmap_8x8(buffer_bitmap_8x8);
        SSD1306_set_bitmap(buffer_bitmap_8x8, 8, 8, 75, 3);

        SSD1306_set_bitmap(wifi_icon_8x8, 8, 8, 65, 3);
        SSD1306_set_text_6x8(FONT_lcd5x7, buffer_ip, 4, 24  );
        SSD1306_display();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

void WIFI_sta_rssi_bitmap_8x8(
    uint8_t *bitmap)
{
    uint8_t wifi_rssi_8x8[64] = { 0 };
    uint8_t rssi_level = WIFI_sta_rssi_level();
    uint16_t i = 0;

    // ESP_LOGI(TAG, "STA RSSI Level is %d", rssi_level);

    for (uint8_t y = 0; y < 8; y++) {
        for (uint8_t x = 0; x < 8; x++) {
            if ((x == 1 && y > 5)
                    || (x == 3 && y > 3 && rssi_level > 0)
                    || (x == 5 && y > 1 && rssi_level > 1)
                    || (x == 7 && rssi_level > 2)) {
                wifi_rssi_8x8[i] = 1;
            }
            i++;
        }
    }

    memcpy(bitmap, &wifi_rssi_8x8, sizeof(uint8_t) * 64);
}