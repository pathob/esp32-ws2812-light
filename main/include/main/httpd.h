#ifndef __MAIN_HTTPD_H__
#define __MAIN_HTTPD_H__

#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_types.h"

#include "libesphttpd/esp.h"
#include "libesphttpd/httpd.h"
#include "libesphttpd/httpd-freertos.h"
#include "libesphttpd/httpdespfs.h"
#include "libesphttpd/cgiwifi.h"
#include "libesphttpd/cgiflash.h"
#include "libesphttpd/auth.h"
#include "libesphttpd/espfs.h"
#include "libesphttpd/captdns.h"
#include "libesphttpd/webpages-espfs.h"
#include "libesphttpd/cgiwebsocket.h"
#include "libesphttpd/route.h"

#include "main/defs.h"
#include "main/stripe.h"

void HTTPD_init();

void ledWebsocketConnect(
    Websock *ws);

void ledWebsocketReceive(
    Websock *ws,
    char *data,
    int len,
    int flags);

void ledWebsocketBroadcast();

#endif
