//
// Created by Kirill Shypachov on 16.09.2025.
//
#include <zephyr/kernel.h>
#include <zephyr/net/http/service.h>
#include <zephyr/net/http/server.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/printk.h>
#include <zephyr/data/json.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include "../global_var.h"

LOG_MODULE_REGISTER(static_resources);


static struct http_resource_detail_static_fs static_fs_resource_detail = {
    .common = {
        .type                              = HTTP_RESOURCE_TYPE_STATIC_FS,
        .bitmask_of_supported_http_methods = BIT(HTTP_GET),
    },
    .fs_path = base_web_ui_fs_path,
};

HTTP_RESOURCE_DEFINE(static_fs_resource, http_static_service, "*", &static_fs_resource_detail);