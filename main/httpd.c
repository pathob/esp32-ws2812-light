#include "main/httpd.h"

static const char *TAG = "HTTPD";

static HttpdFreertosInstance httpd_instance;

static uint8_t *httpd_buffer;

static const HttpdBuiltInUrl builtInUrls[] = {
    ROUTE_REDIRECT("/", "/index.html"),
    ROUTE_REDIRECT("/apple-touch-icon.png", "/launcher-icon-48.png"),
    ROUTE_WS("/websocket/led", ledWebsocketConnect),
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

void ledWebsocketConnect(
    Websock *ws)
{
	ws->recvCb = ledWebsocketReceive;
    cgiWebsocketSend(&httpd_instance.httpdInstance, ws, STRIPE_state() ? "1" : "0", 1, WEBSOCK_FLAG_NONE);
}

void ledWebsocketReceive(
    Websock *ws,
    char *data,
    int len,
    int flags)
{
    ESP_LOGI(TAG, "Received websocket");

    if (len == 1) {
        strncmp(data, "0", 1) == 0 ? STRIPE_off() : STRIPE_on();
    }
}

void ledWebsocketBroadcast()
{
    cgiWebsockBroadcast(&httpd_instance.httpdInstance, "/websocket/led", STRIPE_state() ? "1" : "0", 1, WEBSOCK_FLAG_NONE);
}
