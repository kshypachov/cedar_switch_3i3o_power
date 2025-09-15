//
// Created by Kirill Shypachov on 15.09.2025.
//

#include "http_common.h"

static int updated = 0;

void http_settings_status_set_updated(void) {
    updated = 1;
}

int http_settings_status_get(void) {
    return updated;
}