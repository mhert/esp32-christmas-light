#include <esp_http_server.h>
#include <esp_log.h>
#include "webserver.h"
#include "common.h"

uint32_t state = 0;

httpd_handle_t webserver = NULL;

esp_err_t index_handler(httpd_req_t *req) {
    const char *tmpl =
            "<!DOCTYPE html>\n"
            "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"de\"><head>\n"
            "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
            "    <meta charset=\"utf-8\">\n"
            "    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n"
            "    <meta name=\"viewport\" content=\"width=device-width; initial-scale=1\">\n"
            "    <style>\n"
            "        * {\n"
            "            box-sizing: border-box;\n"
            "            margin: 0;\n"
            "            padding: 0;\n"
            "        }\n"
            "        html {\n"
            "            height: 100%%;\n"
            "            font-family: \"Roboto\", sans-serif;\n"
            "        }\n"
            "        body {\n"
            "            min-height: 100%%;\n"
            "            position: relative;\n"
            "            display: flex;\n"
            "            justify-content: center;\n"
            "            align-items: center;\n"
            "        }\n"
            "        button {\n"
            "            font-size: xx-large;\n"
            "        }\n"
            "    </style>\n"
            "</head>\n"
            "<body><div>%s</div></body>\n"
            "</html>";

    const char *tmpl_link;
    if (state == 0) {
        tmpl_link = "<form>\n"
                    "  <button formaction=\"/on\">Turn On</button>\n"
                    "</form>";
    } else {
        tmpl_link = "<form>\n"
                    "  <button formaction=\"/off\">Turn Off</button>\n"
                    "</form>";;
    }

    char output[strlen(tmpl) + strlen(tmpl_link) + 1];

    sprintf(output, tmpl, tmpl_link);
    httpd_resp_send(req, output, strlen(output));

    return ESP_OK;
}

esp_err_t on_handler(httpd_req_t *req) {
    state = 1;

    gpio_pad_select_gpio(GPIO_NUM_21);
    gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_21, state);

    httpd_resp_set_status(req, "302");
    httpd_resp_set_type(req, "Moved");
    httpd_resp_set_hdr(req, "Location", "/");

    httpd_resp_send(req, "", 0);

    return ESP_OK;
}

esp_err_t off_handler(httpd_req_t *req) {
    state = 0;

    gpio_pad_select_gpio(GPIO_NUM_21);
    gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_21, state);

    httpd_resp_set_status(req, "302");
    httpd_resp_set_type(req, "Moved");
    httpd_resp_set_hdr(req, "Location", "/");

    httpd_resp_send(req, "", 0);

    return ESP_OK;
}

httpd_uri_t index_route = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL,
};

httpd_uri_t on_route = {
        .uri       = "/on",
        .method    = HTTP_GET,
        .handler   = on_handler,
        .user_ctx  = NULL,
};

httpd_uri_t off_route = {
        .uri       = "/off",
        .method    = HTTP_GET,
        .handler   = off_handler,
        .user_ctx  = NULL,
};

void register_and_log_route(httpd_handle_t *webserver, httpd_uri_t *route) {
    static const char *method_strings[] =
            {
#define XX(num, name, string) #string,
                    HTTP_METHOD_MAP(XX)
#undef XX
            };

    ESP_ERROR_CHECK(httpd_register_uri_handler(webserver, route));
    ESP_LOGI(TAG, "handler %s method %s registered", route->uri, method_strings[route->method]);
}

void init_webserver(const tcpip_adapter_ip_info_t *ip) {
    httpd_config_t webserver_config = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&webserver, &webserver_config));

    register_and_log_route(&webserver, &index_route);
    register_and_log_route(&webserver, &on_route);
    register_and_log_route(&webserver, &off_route);

    char port[6];
    sprintf((char *) port, "%d", webserver_config.server_port);
    ESP_LOGI(TAG, "Webserver reachable under http://%s:%s", ip4addr_ntoa(&ip->ip), port);

    ESP_LOGI(TAG, "Done intializing webserver");
}
