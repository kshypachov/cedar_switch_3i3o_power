//
// Created by Kirill Shypachov on 18.09.2025.
//

#include <zephyr/kernel.h>
#include <zephyr/data/json.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/http/service.h>
#include <zephyr/net/http/server.h>
#include <string.h>
#include "../../global_var.h"
#include "../../littlefs/filesystem_helpers.h"

LOG_MODULE_REGISTER(webui_delete);

static int delete_web_ui_files(void) {

    int rc = 0;
    fs_delete_tree(base_web_ui_fs_path);
    fs_unlink(base_web_ui_fs_path);
    rc = fs_mkdir(base_web_ui_fs_path);
    return rc;
}

static int webui_delete_handler(struct http_client_ctx *client,
                            enum http_data_status status,
                            const struct http_request_ctx *request_ctx,
                            struct http_response_ctx *response_ctx,
                            void *user_data) {

    ARG_UNUSED(user_data);

    if (client->method == HTTP_DELETE) {

        if (status == HTTP_SERVER_DATA_ABORTED) {
            return 0;
        }

        if (status == HTTP_SERVER_DATA_FINAL) {

            if (delete_web_ui_files() == 0) {
                response_ctx->status = HTTP_200_OK;
                static const char msg[] = "{\"success\":\"web ui files deleted\"}";
                response_ctx->body = (uint8_t *)msg;
                response_ctx->body_len = sizeof(msg) - 1;
                response_ctx->final_chunk = true;
                return 0;

            }else {
                response_ctx->status = HTTP_500_INTERNAL_SERVER_ERROR;
                static const char msg[] = "{\"error\":\"err deleting web ui files\"}";
                response_ctx->body = (uint8_t *)msg;
                response_ctx->body_len = sizeof(msg) - 1;
                response_ctx->final_chunk = true;
                return 0;
            }
        }
    }else {
        return -1;
    }
    return 0;
}

static struct http_resource_detail_dynamic webui_delete_state = {
    .common = {
        .type = HTTP_RESOURCE_TYPE_DYNAMIC,
        .bitmask_of_supported_http_methods = BIT(HTTP_DELETE),
        .content_type = "application/json"
    },
    .cb = webui_delete_handler,
    .user_data = NULL,
};

/* === Register path for HTTP service only === */
HTTP_RESOURCE_DEFINE(api_webui_delete_state,
                     http_api_service,
                     "/api/webui/delete",
                     &webui_delete_state);