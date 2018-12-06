#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <lwip/apps/sntp.h>
#include <nvs_flash.h>

#include "common.h"
#include "sntp.h"
#include "webserver.h"

#define WIFI_SSID ""
#define WIFI_PASS ""

void on_wifi_connection_established(const tcpip_adapter_ip_info_t *ip) {
    init_webserver(ip);
    init_sntp();
}

esp_err_t event_handler(void *ctx, system_event_t *event) {
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            on_wifi_connection_established(&event->event_info.got_ip.ip_info);
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            /* This is a workaround as ESP32 WiFi libs don't currently
               auto-reassociate. */

            ESP_LOGI(TAG, "Disconnect -> Reboot");
            esp_restart();
            break;

        default:
            break;
    }
    return ESP_OK;
}

extern "C" {
void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());

    tcpip_adapter_init();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_config = {
            .sta = {
                    {.ssid = WIFI_SSID,},
                    {.password = WIFI_PASS,}
            },
    };

    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_start());
}
}
