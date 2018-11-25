#include "main/mqtt.h"

static const char *TAG = "MQTT";

static char _mqtt_device_name[12];
static esp_mqtt_client_handle_t _mqtt_client;

static esp_err_t MQTT_event_handler(
    esp_mqtt_event_handle_t event);

static void MQTT_receive(
    esp_mqtt_event_handle_t event);

void MQTT_task(
    void *pvParameters)
{
    CONNECTIVITY_wait(WIFI_STA_CONNECTED);

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(_mqtt_device_name, "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    const esp_mqtt_client_config_t mqtt_cfg = {
        .host = CONFIG_MQTT_HOST,
        .port = CONFIG_MQTT_PORT,
        .username = CONFIG_MQTT_USERNAME,
        .password = CONFIG_MQTT_PASSWORD,
        .event_handle = MQTT_event_handler,
    };

    _mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(_mqtt_client);

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

// TODO: Respect MQTT connection status
void MQTT_topic_led_broadcast()
{
    CONNECTIVITY_wait(MQTT_CONNECTED);

    char mqtt_topic[24];
    sprintf(mqtt_topic, "%s/led/state", _mqtt_device_name);

    esp_mqtt_client_publish(_mqtt_client, mqtt_topic, STRIPE_state() ? "1" : "0", 1, 2, 1);
}

static esp_err_t MQTT_event_handler(
    esp_mqtt_event_handle_t event)
{
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED: {
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            CONNECTIVITY_set(MQTT_CONNECTED);

            char mqtt_topic[24];
            sprintf(mqtt_topic, "%s/led", _mqtt_device_name);

            esp_mqtt_client_subscribe(_mqtt_client, mqtt_topic, 2);
            break;
        }
        case MQTT_EVENT_DISCONNECTED: {
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            CONNECTIVITY_clear(MQTT_CONNECTED);
            break;
        }
        case MQTT_EVENT_DATA: {
            ESP_LOGD(TAG, "MQTT_EVENT_DATA");
            MQTT_receive(event);
            break;
        }
        default:
            break;
    }

    return ESP_OK;
}

static void MQTT_receive(
    esp_mqtt_event_handle_t event)
{
    char mqtt_topic[24];
    uint8_t mqtt_topic_len;
    mqtt_topic_len = sprintf(mqtt_topic, "%s/led", _mqtt_device_name);

    ESP_LOGI(TAG, "Received Topic: %.*s (%d), Message: %.*s (%d)", event->topic_len, event->topic, event->topic_len, event->data_len, event->data, event->data_len);

    if (mqtt_topic_len == event->topic_len) {
        if (strncmp(mqtt_topic, event->topic, event->topic_len) == 0) {
            if (event->data_len == 1) {
                strncmp((char *) event->data, "0", 1) == 0 ? STRIPE_off() : STRIPE_on();
            }
        }
    }
}
