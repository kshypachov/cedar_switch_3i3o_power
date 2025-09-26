//
// Created by Kirill Shypachov on 14.09.2025.
//

#include "io.h"
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <string.h>
#include "../zbus_topics.h"
#include "../settings_topics.h"


LOG_MODULE_REGISTER(io);

/* Стек для нового потока */
#define IO_TASK_STACK_SIZE 2048
#define IO_TASK_PRIORITY   2

#define RELAY1_NODE DT_ALIAS(relay1)
#define RELAY2_NODE DT_ALIAS(relay2)
#define RELAY3_NODE DT_ALIAS(relay3)

K_THREAD_STACK_DEFINE(io_task_stack, IO_TASK_STACK_SIZE);
static struct k_thread io_task_thread_data;

relays_def_state relays_default_state;




/* ====== DT relays (gpio-leds children) ====== */
static const struct gpio_dt_spec relay1 = GPIO_DT_SPEC_GET(RELAY1_NODE, gpios);
static const struct gpio_dt_spec relay2 = GPIO_DT_SPEC_GET(RELAY2_NODE, gpios);
static const struct gpio_dt_spec relay3 = GPIO_DT_SPEC_GET(RELAY3_NODE, gpios);
//static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(RELAY1_NODE, gpios);


static void write_relays_once(uint8_t state) {
    gpio_pin_set_dt(&relay1, (state & 1)? 1 : 0);
    gpio_pin_set_dt(&relay2, (state & 2)? 1 : 0);
    gpio_pin_set_dt(&relay3, (state & 4)? 1 : 0);
}

void io_task(void *a, void *b, void *c) {

    ARG_UNUSED(a);
    ARG_UNUSED(b);
    ARG_UNUSED(c);

    /* Инициализация GPIO как выходов (логически неактивны на старте) */
    if (!device_is_ready(relay1.port) || !device_is_ready(relay2.port) || !device_is_ready(relay3.port)) {
        /* Порт не готов – нечего делать */
        return;
    }

    (void)gpio_pin_configure_dt(&relay1, GPIO_OUTPUT_INACTIVE);
    (void)gpio_pin_configure_dt(&relay2, GPIO_OUTPUT_INACTIVE);
    (void)gpio_pin_configure_dt(&relay3, GPIO_OUTPUT_INACTIVE);

    uint8_t applied_state = 0x00; /* force first apply */
    write_relays_once(applied_state);

    mqtt_status_msg_t mqtt_status_msg = {
        false
    };
    settings_load_one(mqtt_enabled_settings, &mqtt_status_msg.enabled, sizeof(mqtt_status_msg.enabled));


    while (1) {
        outputs_msg_t msg;
        if (zbus_chan_read(&outputs_zbus_topik, &msg, K_NO_WAIT) == 0) {
            // if mqtt function is desabled, of function of default relay state is disabled - not follow for mqtt connection
            if ((mqtt_status_msg.enabled == false) || (relays_default_state.enabled == false)){
                if (msg.state != applied_state) {
                    applied_state = msg.state;
                    write_relays_once(applied_state);
                }
            }else{
                // if mqtt_status_msg.enabled == true and relays_default_state.enabled == true
                // its mean mqtt function is enabled and we set default relay state if mqtt connection lost
                // read mqtt conection status
                zbus_chan_read(&mqtt_stat_zbus_topik, &mqtt_status_msg, K_NO_WAIT);
                // if mqtt connection is lost - set default relay state
                if (mqtt_status_msg.connected == false) {
                    msg.state = (uint8_t)(relays_default_state.relay1  << 0) |
                                (relays_default_state.relay2 ? 1 : 0 ) << 1 |
                                (relays_default_state.relay3 ? 1 : 0) << 2;
                    if (msg.state != applied_state) {
                        applied_state = msg.state;
                        write_relays_once(applied_state);
                    }
                // if mqtt connection is established - set relays state from mqtt
                }else {
                    if (msg.state != applied_state) {
                        applied_state = msg.state;
                        write_relays_once(applied_state);
                    }
                }
            }
        }
    k_sleep(K_MSEC(250));
    }
}

void io_init(void) {
    LOG_INF("Start io init");

    if ( 0 > settings_load_one(def_relays_state_enable_settings_topik, &relays_default_state.enabled, sizeof(relays_default_state.enabled))) {
        LOG_ERR("Failed to load topik %s init default", def_relays_state_enable_settings_topik);
        relays_default_state.enabled = false;
        settings_save_one(def_relays_state_enable_settings_topik, &relays_default_state.enabled, sizeof(relays_default_state.enabled));
    }
    if ( 0 > settings_load_one(def_relays1_state_settings_topik, &relays_default_state.relay1, sizeof(relays_default_state.relay1))) {
        LOG_ERR("Failed to load topik %s init default", def_relays1_state_settings_topik);
        relays_default_state.relay1 = false;
        settings_save_one(def_relays1_state_settings_topik, &relays_default_state.relay1, sizeof(relays_default_state.relay1));
    }
    if ( 0 > settings_load_one(def_relays2_state_settings_topik, &relays_default_state.relay2, sizeof(relays_default_state.relay2))) {
        LOG_ERR("Failed to load topik %s init default", def_relays2_state_settings_topik);
        relays_default_state.relay2 = false;
        settings_save_one(def_relays2_state_settings_topik, &relays_default_state.relay2, sizeof(relays_default_state.relay2));
    }
    if ( 0 > settings_load_one(def_relays3_state_settings_topik, &relays_default_state.relay3, sizeof(relays_default_state.relay3))) {
        LOG_ERR("Failed to load topik %s init default", def_relays3_state_settings_topik);
        relays_default_state.relay3 = false;
        settings_save_one(def_relays3_state_settings_topik, &relays_default_state.relay3, sizeof(relays_default_state.relay3));
    }

    k_tid_t tid = k_thread_create(&io_task_thread_data,
                              io_task_stack,
                              K_THREAD_STACK_SIZEOF(io_task_stack),
                              io_task,
                              NULL, NULL, NULL,
                              IO_TASK_PRIORITY,
                              0,
                              K_NO_WAIT);

    k_thread_name_set(tid, "io_task");
}