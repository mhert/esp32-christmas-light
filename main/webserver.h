#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include <esp_http_server.h>
#include <esp_wifi.h>

void init_webserver(const tcpip_adapter_ip_info_t *ip);

#endif //WEBSERVER_WEBSERVER_H
