#include "main/mqtt.h"

static const char *TAG = "MQTT";

static char _mqtt_device_name[12] = { 0 };

void MQTT_init_handler() {
    CONNECTIVITY_device_id(_mqtt_device_name);
}

void MQTT_connected_handler(
    esp_mqtt_event_handle_t event)
{
    char mqtt_topic[24];
    sprintf(mqtt_topic, "%.12s/led", _mqtt_device_name);
    MQTT_subscribe(mqtt_topic, 2);
}

void MQTT_data_handler(
    esp_mqtt_event_handle_t event)
{
    char mqtt_topic[24];
    uint8_t mqtt_topic_len;
    mqtt_topic_len = sprintf(mqtt_topic, "%.12s/led", _mqtt_device_name);

    ESP_LOGI(TAG, "Received Topic: %.*s (%d), Message: %.*s (%d)", event->topic_len, event->topic, event->topic_len, event->data_len, event->data, event->data_len);

    if (mqtt_topic_len == event->topic_len) {
        if (strncmp(mqtt_topic, event->topic, event->topic_len) == 0) {
            if (event->data_len == 1) {
                strncmp((char *) event->data, "0", 1) == 0 ? STRIPE_off() : STRIPE_on();
            }
        }
    }
}

// TODO: Respect MQTT connection status
void MQTT_topic_led_broadcast()
{
    MQTT_connectivity_wait();

    char mqtt_topic[24];
    sprintf(mqtt_topic, "%.12s/led/state", _mqtt_device_name);

    MQTT_publish(mqtt_topic, STRIPE_state() ? "1" : "0", 1, 2, 1);
}
