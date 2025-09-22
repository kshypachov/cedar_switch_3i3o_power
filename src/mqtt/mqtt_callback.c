//
// Created by Kirill Shypachov on 20.09.2025.
//

#include "mqtt_callback.h"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include "../zbus_topics.h"
#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(mqtt_callback);




void mqtt_evt_handler(struct mqtt_client *const client,
              const struct mqtt_evt *evt)
{
    int err;
    static mqtt_status_msg_t mqtt_conn_stat = {
        0,
        true,
        true
    };

	LOG_DBG("mqtt_evt_handler");

    switch (evt->type) {
        case MQTT_EVT_CONNACK:
            if (evt->result != 0) {
                LOG_ERR("MQTT connect failed %d", evt->result);
                break;
            }

            mqtt_conn_stat.connected = true;
            mqtt_conn_stat.seq = k_uptime_get();
			zbus_chan_pub(&mqtt_stat_zbus_topik, &mqtt_conn_stat, K_NO_WAIT);

            LOG_INF("MQTT client connected!");

#if defined(CONFIG_MQTT_VERSION_5_0)
            if (evt->param.connack.prop.rx.has_topic_alias_maximum &&
                evt->param.connack.prop.topic_alias_maximum > 0) {
                LOG_INF("Topic aliases allowed by the broker, max %u.",
                    evt->param.connack.prop.topic_alias_maximum);

                aliases_enabled = true;
                } else {
                    LOG_INF("Topic aliases disallowed by the broker.");
                }
#endif

#if defined(CONFIG_LOG_BACKEND_MQTT)
            log_backend_mqtt_client_set(client);
#endif

            break;

        case MQTT_EVT_DISCONNECT:
            LOG_INF("MQTT client disconnected %d", evt->result);

            mqtt_conn_stat.connected = false;
            mqtt_conn_stat.seq = k_uptime_get();
			zbus_chan_pub(&mqtt_stat_zbus_topik, &mqtt_conn_stat, K_NO_WAIT);

#if defined(CONFIG_LOG_BACKEND_MQTT)
            log_backend_mqtt_client_set(NULL);
#endif

            break;

        case MQTT_EVT_PUBACK:
            if (evt->result != 0) {
                LOG_ERR("MQTT PUBACK error %d", evt->result);
                break;
            }

            LOG_INF("PUBACK packet id: %u", evt->param.puback.message_id);

            break;

        case MQTT_EVT_PUBLISH:
            if (evt->param.publish.message.topic.qos == MQTT_QOS_1_AT_LEAST_ONCE)
            {
                const struct mqtt_puback_param publish_real_param = {
                    .message_id = evt->param.publish.message_id,
                };
                int qos1_ack_rc = mqtt_publish_qos1_ack(client, &publish_real_param);
                if (qos1_ack_rc != 0)
                {
                    LOG_ERR("MQTT PUBLISH QOS1 ack failed");
                }
            }

            if (evt->param.publish.message.topic.qos == MQTT_QOS_2_EXACTLY_ONCE)
            {
                const struct mqtt_pubrec_param pubrec_real_param = {
                    .message_id = evt->param.publish.message_id,
                };
                int qos2_ack_rc = mqtt_publish_qos2_receive(client, &pubrec_real_param);
                if (qos2_ack_rc != 0)
                {
                    LOG_ERR("MQTT PUBLISH QOS2 ack failed");
                }
            }

            break;

        case MQTT_EVT_PUBREC:
            if (evt->result != 0) {
                LOG_ERR("MQTT PUBREC error %d", evt->result);
                break;
            }

            LOG_INF("PUBREC packet id: %u", evt->param.pubrec.message_id);

            const struct mqtt_pubrel_param rel_param = {
                .message_id = evt->param.pubrec.message_id
            };

            err = mqtt_publish_qos2_release(client, &rel_param);
            if (err != 0) {
                LOG_ERR("Failed to send MQTT PUBREL: %d", err);
            }

            break;

        case MQTT_EVT_PUBCOMP:
            if (evt->result != 0) {
                LOG_ERR("MQTT PUBCOMP error %d", evt->result);
                break;
            }

            LOG_INF("PUBCOMP packet id: %u",
                evt->param.pubcomp.message_id);

            break;

        case MQTT_EVT_PINGRESP:
            LOG_INF("PINGRESP packet");
            break;


        default:
            break;
    }
}


