//
// Created by Kirill Shypachov on 14.09.2025.
//
#include "http_server_init.h"
#include <zephyr/net/http/service.h>
#include <zephyr/net/http/server.h>
#include <zephyr/logging/log.h>

static uint16_t http_service_port = 80;

LOG_MODULE_REGISTER(http_server_init, LOG_LEVEL_INF);


HTTP_SERVICE_DEFINE(my_service, "0.0.0.0", &http_service_port, 1, 10, NULL, NULL);

void app_http_server_init(void) {
    http_server_start();
}