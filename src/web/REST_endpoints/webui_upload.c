//
// Created by Kirill Shypachov on 19.09.2025.
//
#include <zephyr/fs/fs.h>
#include <zephyr/net/http/server.h>
#include <zephyr/net/http/service.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <errno.h>
#include "../../global_var.h"
#include "../http_common.h"

LOG_MODULE_REGISTER(webui_upload);

#define WEBUI_ROOT base_web_ui_fs_path





#define CHUNK_BUF_CAP 512

static int webui_chunk_put(struct http_client_ctx *client,
                            enum http_data_status status,
                            const struct http_request_ctx *request_ctx,
                            struct http_response_ctx *response_ctx,
                            void *user_data) {
    ARG_UNUSED(user_data);

    static char post_request_buff [CHUNK_BUF_CAP] = "\0";
    static size_t cursor;

    if (client->method == HTTP_POST) {

        if (status == HTTP_SERVER_DATA_ABORTED) {
            cursor = 0;
            return 0;
        }

        if (request_ctx->data_len + cursor > sizeof(post_request_buff)) {
            cursor = 0;
            return -ENOMEM;
        }

        memcpy(post_request_buff + cursor, request_ctx->data, request_ctx->data_len);
        cursor += request_ctx->data_len;

        if (status == HTTP_SERVER_DATA_FINAL) {
            return http_upload(client, status, request_ctx, response_ctx, WEBUI_ROOT, cursor, post_request_buff, CHUNK_BUF_CAP);
        }
        return 0;

    } else {
        return -EINVAL;
    }
}


static struct http_resource_detail_dynamic webui_put_detail = {
    .common = {
        .type = HTTP_RESOURCE_TYPE_DYNAMIC,
        .bitmask_of_supported_http_methods = BIT(HTTP_POST),
        //.content_type = "text/plain",
    },
    .cb = webui_chunk_put,
};
HTTP_RESOURCE_DEFINE(webui_put_resource, http_api_service,
                     "/api/webui/upload", &webui_put_detail);