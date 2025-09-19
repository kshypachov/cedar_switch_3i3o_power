//
// Created by Kirill Shypachov on 19.09.2025.
//
#include <zephyr/kernel.h>
#include <zephyr/data/json.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/http/service.h>
#include <zephyr/net/http/server.h>
#include <zephyr/fs/fs.h>
#include <string.h>
#include "../../global_var.h"

LOG_MODULE_REGISTER(webui_mkdir);

typedef struct {
    char mkdir_path[128];
} mkdir_req_t;

static const struct json_obj_descr mkdir_descr[] = {
    JSON_OBJ_DESCR_PRIM_NAMED( mkdir_req_t, "path", mkdir_path, JSON_TOK_STRING_BUF),
};

static int mkdir_web_ui_files(const char *path) {
    char path_to_create[256] = {0};
    int ret = 0;

    if (path[0] == '/') return -1;
    if (!path || !*path) return -1;

    int n = snprintf(path_to_create, sizeof(path_to_create), "%s/%s", base_web_ui_fs_path, path);
    if (n < 0 || (size_t)n >= sizeof(path_to_create)) return -1;
    ret = fs_mkdir(path_to_create);
    return ret;
}

static int mkdir_web_ui_handler(struct http_client_ctx *client,
                            enum http_data_status status,
                            const struct http_request_ctx *request_ctx,
                            struct http_response_ctx *response_ctx,
                            void *user_data)
{
    ARG_UNUSED(user_data);

    static char resp_buf[256] = "\0";
    static char post_request_buff [256] = "\0";
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
            mkdir_req_t tmp = {0};
            const int expected = BIT_MASK(ARRAY_SIZE(mkdir_descr));
            int ret = json_obj_parse(post_request_buff, cursor, mkdir_descr, ARRAY_SIZE(mkdir_descr), &tmp);

            cursor = 0;
            if (ret != expected) {
                /* Можно различать: <0 — ошибка парсера, иначе — битовая маска отсутствующих полей */
                response_ctx->status = HTTP_400_BAD_REQUEST;
                static const char msg[] = "{\"error\":\"invalid json or missing fields\"}";
                response_ctx->body = (uint8_t *)msg;
                response_ctx->body_len = sizeof(msg) - 1;
                response_ctx->final_chunk = true;

                return 0;
            }
            if (mkdir_web_ui_files(tmp.mkdir_path) >= 0) {
                response_ctx->status = HTTP_200_OK;
                static const char msg[] = "{\"success\":\"web ui dir created\"}";
                response_ctx->body = (uint8_t *)msg;
                response_ctx->body_len = sizeof(msg) - 1;
                response_ctx->final_chunk = true;

                return 0;

            }else{
                response_ctx->status = HTTP_500_INTERNAL_SERVER_ERROR;
                static const char msg[] = "{\"error\":\"err creating web ui dir\"}";
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

static struct http_resource_detail_dynamic webui_mkdir_state = {
    .common = {
        .type = HTTP_RESOURCE_TYPE_DYNAMIC,
        .bitmask_of_supported_http_methods = BIT(HTTP_POST),
        .content_type = "application/json"
    },
    .cb = mkdir_web_ui_handler,
    .user_data = NULL,
};

/* === Register path for HTTP service only === */
HTTP_RESOURCE_DEFINE(api_webui_mkdir_state,
                     http_api_service,
                     "/api/webui/mkdir",
                     &webui_mkdir_state);