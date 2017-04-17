#include <wifi.h>
#include <ota.h>

void WIFI_connected()
{
    printf("Wifi connected\n");
}

void WIFI_disconnected()
{
    printf("Wifi disconnected\n");
}

void app_main()
{
    nvs_flash_init();
    WIFI_init(NULL);

    // TODO: How to decide if update should be started?
    xTaskCreate(&OTA_task, "OTA_task", 2048, NULL, 10, NULL);

    /*
    esp_bt_controller_init();

    if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
        return;
    }

    ESP_LOGI("TAG", "TEST");

    xTaskCreate(&bleAdvtTask, "bleAdvtTask", 2048, (void* ) 0, 10, NULL);
//    xTaskCreate(&wifiTask,    "wifiTask",    2048, (void* ) 1, 10, NULL);
     */
}
