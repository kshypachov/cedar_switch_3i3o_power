//
// Created by Kirill Shypachov on 20.09.2025.
//

#ifndef CEDAR_SWITCH_3IN3OUT_POWER_MQTT_HELPERS_H
#define CEDAR_SWITCH_3IN3OUT_POWER_MQTT_HELPERS_H

#include <zephyr/net/mqtt.h>

typedef enum{
    INPUT_SENSOR			= 1,
    OUTPUT_DEVICE			= 2,
    ENERGY_SENSOR			= 3,
    POWER_SENSOR			= 4,
    VOLTAGE_SENSOR			= 5,
    POWER_FACTOR_SENSOR		= 6,
    CURRENT_SENSOR 			= 7,
    APPARENT_POWER_SENSOR	= 8,
    VOLTAGE_DIAGNOSTIC_BATT_SENSOR	= 9,
    VOLTAGE_DIAGNOSTIC_POW_SUPL_SENSOR	= 10,
}mqtt_ha_dev_type_t;

int set_device_id(const uint8_t* id, unsigned const int id_len);
const char *get_device_id(void);
int set_device_conf_ip(const char *ip_str);
const char *get_device_conf_ip(void);

#endif //CEDAR_SWITCH_3IN3OUT_POWER_MQTT_HELPERS_H