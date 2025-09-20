//
// Created by Kirill Shypachov on 14.09.2025.
//

#ifndef CEDAR_SWITCH_3IN3OUT_POWER_SETTINGS_TOPICS_H
#define CEDAR_SWITCH_3IN3OUT_POWER_SETTINGS_TOPICS_H
#include <stdbool.h>

#define def_relays_state_enable_settings_topik "/settings/relays/default/enabled"
#define def_relays1_state_settings_topik      "/settings/relays/default/1"
#define def_relays2_state_settings_topik      "/settings/relays/default/2"
#define def_relays3_state_settings_topik      "/settings/relays/default/3"

typedef struct  {
    bool enabled;
    bool relay1;
    bool relay2;
    bool relay3;
} relays_def_state;


#define mqtt_enabled_settings "/settings/mqtt/enabled"
#define mqtt_host_settings    "/settings/mqtt/host"
#define mqtt_port_settings    "/settings/mqtt/port"
#define mqtt_user_settings    "/settings/mqtt/user"
#define mqtt_pass_settings    "/settings/mqtt/pass"

typedef struct {
    bool enabled;
    char host[128];
    uint16_t port;
    char user[128];
    char pass[128];
} mqtt_settings_t;


#endif //CEDAR_SWITCH_3IN3OUT_POWER_SETTINGS_TOPICS_H