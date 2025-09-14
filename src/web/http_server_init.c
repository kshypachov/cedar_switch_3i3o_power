//
// Created by Kirill Shypachov on 14.09.2025.
//
#include "http_server_init.h"
#include <zephyr/net/http/service.h>
#include <zephyr/net/http/server.h>

static uint16_t http_service_port = 80;

HTTP_SERVICE_DEFINE(my_service, "0.0.0.0", &http_service_port, 1, 10, NULL, NULL);

void app_http_server_init(void) {
    http_server_start();
}