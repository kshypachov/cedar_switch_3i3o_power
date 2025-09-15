//
// Created by Kirill Shypachov on 15.09.2025.
//

#ifndef CEDAR_SWITCH_3IN3OUT_POWER_ZBUS_TOPICS_H
#define CEDAR_SWITCH_3IN3OUT_POWER_ZBUS_TOPICS_H
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <stdbool.h>

#define outputs_zbus_topik outputs_topic
#define inputs_zbus_topik inputs_topic
#define mqtt_status_zbus_topik mqtt_status_topic

//structure for relay state exchange
struct outputs_msg {
    uint32_t seq;   /* уникальный номер или счётчик сообщения */
    uint8_t state;  /* биты реле */
};

struct inputs_msg {
    uint32_t seq;  /**/
    uint8_t state;  /**/
};

struct mqtt_status_msg {
    uint32_t seq;
    bool enabled;
    bool connected;
};

#endif //CEDAR_SWITCH_3IN3OUT_POWER_ZBUS_TOPICS_H