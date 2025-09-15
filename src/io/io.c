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
#include <string.h>
#include "../zbus_topics.h"

LOG_MODULE_REGISTER(io, LOG_LEVEL_INF);

/* Стек для нового потока */
#define IO_TASK_STACK_SIZE 2048
#define IO_TASK_PRIORITY   5

K_THREAD_STACK_DEFINE(io_task_stack, IO_TASK_STACK_SIZE);
static struct k_thread io_task_thread_data;

ZBUS_CHAN_DECLARE(outputs_topic);

/* ====== DT relays (gpio-leds children) ====== */
static const struct gpio_dt_spec relay1 = GPIO_DT_SPEC_GET(DT_NODELABEL(relay1), gpios);
static const struct gpio_dt_spec relay2 = GPIO_DT_SPEC_GET(DT_NODELABEL(relay2), gpios);
static const struct gpio_dt_spec relay3 = GPIO_DT_SPEC_GET(DT_NODELABEL(relay3), gpios);


static void write_relays_once(uint8_t state) {
    gpio_pin_set_dt(&relay1, state & 1);
    gpio_pin_set_dt(&relay2, state & 2);
    gpio_pin_set_dt(&relay3, state & 4);
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


    while (1) {
        struct outputs_msg msg;
        if (zbus_chan_read(&outputs_zbus_topik, &msg, K_NO_WAIT) == 0) {
            if (msg.state != applied_state) {
                applied_state = msg.state;
                write_relays_once(applied_state);
            }
        }
    k_sleep(K_MSEC(100));
    }

}


void io_init(void) {

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