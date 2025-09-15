# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(cedar_switch_3in3out_power)

target_sources(app PRIVATE
        src/main.c
        src/zbus_topics.c
        src/web/http_server_init.c
        src/mqtt/mqtt.c
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
zephyr_linker_section(NAME http_resource_desc_my_service
        KVMA RAM_REGION GROUP RODATA_REGION)