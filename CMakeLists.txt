# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(cedar_switch_3in3out_power)

target_sources(app PRIVATE
        src/main.c
        src/test_functions.c # to delete
        src/littlefs/filesystem_helpers.c
        src/zbus_topics.c
        src/msgq_topiks.c
        src/web/http_common.c
        src/web/REST_endpoints/settings_status.c
        src/web/REST_endpoints/default_relay_state.c
        src/web/REST_endpoints/mqtt_settings.c
        src/web/REST_endpoints/relays_state.c
        src/web/REST_endpoints/system_reboot.c
        src/web/REST_endpoints/webui_delete.c
        src/web/REST_endpoints/webui_mkdir.c
        src/web/REST_endpoints/webui_upload.c
        src/web/static_resources.c
        src/web/http_server_init.c
        src/mqtt/ha_mqtt.c
        src/mqtt/ha_mqtt_callback.c
        src/mqtt/ha_mqtt_helpers.c
        src/mqtt/ha_mqtt_adapter.c
        src/mqtt/ha_task.c
        src/littlefs/littlefs_mount.c
        src/io/io.c
#        src/io.c
#        src/fs_mount.c
#        src/nvs_mount.c
#        src/app/web.c
#        src/app/mongoose.c
#        src/app/home_assistant_mqtt.c
#        src/app/rest_api.c
)

zephyr_linker_sources(SECTIONS sections-rom.ld)

zephyr_linker_section(NAME http_resource_desc_http_static_service
        KVMA RAM_REGION GROUP RODATA_REGION)

zephyr_linker_section(NAME http_resource_desc_http_api_service
        KVMA RAM_REGION GROUP RODATA_REGION)