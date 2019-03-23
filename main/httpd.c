#include "main/httpd.h"

static const char *TAG = "HTTPD";

static HttpdFreertosInstance httpd_instance;

static uint8_t *httpd_buffer;

static const HttpdBuiltInUrl builtInUrls[] = {
    ROUTE_REDIRECT("/", "/index.html"),
    ROUTE_REDIRECT("/apple-touch-icon.png", "/launcher-icon-48.png"),
    ROUTE_WS("/websocket/led", HTTPD_websocket_led_connect),
    ROUTE_FILESYSTEM(),
    ROUTE_END()
};

void HTTPD_init()
{
    ESP_LOGI(TAG, "Starting...");
    espFsInit((void*)(webpages_espfs_start));

    uint8_t maxConnections = 24;
    httpd_buffer = malloc(sizeof(RtosConnType) * maxConnections);

    httpdFreertosInit(
        &httpd_instance,
        builtInUrls,
        80,
        httpd_buffer,
        maxConnections,
        HTTPD_FLAG_NONE);

    httpdFreertosStart(&httpd_instance);
}

void HTTPD_websocket_led_connect(
    Websock *ws)
{
	ws->recvCb = HTTPD_websocket_led_receive;
    cgiWebsocketSend(&httpd_instance.httpdInstance, ws, STRIPE_get_state() ? "1" : "0", 1, WEBSOCK_FLAG_NONE);
}

void HTTPD_websocket_led_receive(
    Websock *ws,
    char *data,
    int len,
    int flags)
{
    ESP_LOGI(TAG, "Received websocket");

    if (len == 1) {
        uint8_t state = 0;

        if (strncmp(data, "1", 1) == 0) {
            state = 1;
        }

        STRIPE_set(&state, NULL, NULL, 1000);
    }
}

void HTTPD_websocket_led_broadcast(
    char *data,
    uint8_t data_len)
{
    cgiWebsockBroadcast(&httpd_instance.httpdInstance, "/websocket/led", data, data_len, WEBSOCK_FLAG_NONE);
}
