//
// Created by Kirill Shypachov on 20.09.2025.
//

#include "mqtt_callback.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mqtt_callback);

void mqtt_evt_handler(struct mqtt_client *const client,
              const struct mqtt_evt *evt)
{
    int err;
    static bool connected = false;

    switch (evt->type) {
        case MQTT_EVT_CONNACK:
            if (evt->result != 0) {
                LOG_ERR("MQTT connect failed %d", evt->result);
                break;
            }

            connected = true;
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

            connected = false;
            //clear_fds();

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
