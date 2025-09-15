//
// Created by Kirill Shypachov on 15.09.2025.
//
#include "zbus_topics.h"

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <stdbool.h>

#include "sys/socket.h"

// Канал zbus с состоянием выходов
ZBUS_CHAN_DEFINE(outputs_zbus_topik,            /* имя канала */
                 struct outputs_msg,       /* тип сообщения */
                 NULL, NULL,               /* callback до/после публикации (можно NULL) */
                 ZBUS_OBSERVERS_EMPTY,     /* список наблюдателей */
                 ZBUS_MSG_INIT(0, 0));     /* начальное значение */

// Канал zbus с состоянием входов
ZBUS_CHAN_DEFINE(inputs_zbus_topik,            /* имя канала */
                 struct outputs_msg,       /* тип сообщения */
                 NULL, NULL,               /* callback до/после публикации (можно NULL) */
                 ZBUS_OBSERVERS_EMPTY,     /* список наблюдателей */
                 ZBUS_MSG_INIT(0, 0));     /* начальное значение */

// Канал zbus с состоянием MQTT
ZBUS_CHAN_DEFINE(mqtt_stat_zbus_topik,            /* имя канала */
                 struct mqtt_status_msg,       /* тип сообщения */
                 NULL, NULL,               /* callback до/после публикации (можно NULL) */
                 ZBUS_OBSERVERS_EMPTY,     /* список наблюдателей */
                 ZBUS_MSG_INIT(
                     .seq = 0,
                     .enabled = false,
                     .connected = false));     /* начальное значение */

