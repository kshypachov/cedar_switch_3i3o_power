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
#define mqtt_stat_zbus_topik mqtt_status_topic

ZBUS_CHAN_DECLARE(outputs_zbus_topik);
ZBUS_CHAN_DECLARE(inputs_zbus_topik);
ZBUS_CHAN_DECLARE(mqtt_stat_zbus_topik);

//structure for relay state exchange
typedef struct {
    uint32_t seq;   /* уникальный номер или счётчик сообщения */
    uint8_t state;  /* биты реле */
} outputs_msg_t;

typedef struct {
    uint32_t seq;  /**/
    uint8_t state;  /**/
} inputs_msg_t;

typedef struct  {
    uint64_t seq;
    bool enabled;
    bool connected;
}mqtt_status_msg_t;

#endif //CEDAR_SWITCH_3IN3OUT_POWER_ZBUS_TOPICS_H