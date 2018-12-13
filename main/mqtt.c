#include "main/mqtt.h"

static const char *TAG = "MQTT";

static char _mqtt_device_name[12] = { 0 };

void MQTT_init_handler() {
    CONNECTIVITY_device_id(_mqtt_device_name);
}

void MQTT_connected_handler(
    esp_mqtt_event_handle_t event)
{
    char mqtt_topic[32];

    // DEVICE_ID/socket/switch
    // DEVICE_ID/socket/status

    // DEVICE_ID/rgb/switch
    // DEVICE_ID/rgb/status

    // DEVICE_ID/plants

    // Subscribe and directly unsubscribe to/from rgb state topic to get an initial state
    sprintf(mqtt_topic, "%.12s/rgb/status", _mqtt_device_name);
    MQTT_subscribe(mqtt_topic, MQTT_QOS_EXACTLY_ONCE);
    MQTT_unsubscribe(mqtt_topic);

    // Subscribe to rgb switch topic
    sprintf(mqtt_topic, "%.12s/rgb/switch", _mqtt_device_name);
    MQTT_subscribe(mqtt_topic, MQTT_QOS_EXACTLY_ONCE);
}

void MQTT_data_handler(
    esp_mqtt_event_handle_t event)
{
    ESP_LOGI(TAG, "Received Topic: %.*s (%d), Message: %.*s (%d)", event->topic_len, event->topic, event->topic_len, event->data_len, event->data, event->data_len);

    char mqtt_topic[32];
    uint8_t mqtt_topic_len;

    // Check whether a new rgb switch command has been received
    mqtt_topic_len = sprintf(mqtt_topic, "%.12s/rgb/switch", _mqtt_device_name);
    if (strncmp(mqtt_topic, event->topic, mqtt_topic_len)) {
        STRIPE_set_json(event->data);
    }

    // Check whether the initial rgb state has been received
    mqtt_topic_len = sprintf(mqtt_topic, "%.12s/rgb/status", _mqtt_device_name);
    if (strncmp(mqtt_topic, event->topic, mqtt_topic_len)) {
        STRIPE_set_json(event->data);
    }
}

void MQTT_publish_rgb_status(
    char *data,
    uint8_t data_len)
{
    MQTT_connectivity_wait();

    char mqtt_topic[32];
    sprintf(mqtt_topic, "%.12s/rgb/status", _mqtt_device_name);
    MQTT_publish(mqtt_topic, data, data_len, MQTT_QOS_EXACTLY_ONCE, 1);
}
