//
// Created by Kirill Shypachov on 14.09.2025.
//
#include "http_server_init.h"
#include <zephyr/net/http/service.h>
#include <zephyr/net/http/server.h>
#include <zephyr/logging/log.h>

static uint16_t http_static_service_port = 80;
static uint16_t http_api_service_port = 8080;

LOG_MODULE_REGISTER(http_server_init);


HTTP_SERVICE_DEFINE(http_static_service, "0.0.0.0", &http_static_service_port, 1, 10, NULL, NULL);

HTTP_SERVICE_DEFINE(http_api_service, "0.0.0.0", &http_api_service_port, 1, 10, NULL, NULL);

void app_http_server_init(void) {
    http_server_start();
}