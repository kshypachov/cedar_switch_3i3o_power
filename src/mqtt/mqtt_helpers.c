//
// Created by Kirill Shypachov on 20.09.2025.
//

#include "mqtt_helpers.h"
#include "../global_var.h"
#include <errno.h>
#include <arpa/inet.h>

#define ha_universal_config_topik_template	 "%s/%s/%s_%s/%s%u/config"

#define dev_system						"cedar"
#define dev_common_name					"CedarSwitch"
#define dev_manufacturer_name			"Manufacturer"
#define dev_model_name					"CedarSwitch-3R3I-POWER"

#define component_sensor				"sensor"
#define component_binary_sensor			"binary_sensor"
#define component_switch				"switch"

#define component_input					"input"
#define component_input_name			"Вхід"

#define dev_class_switch				component_switch
#define dev_class_switch_name			"Перемикач"

#define component_battery				"battery"
#define component_battery_name  		"Батарея"

#define component_power_supply			"power_supply"
#define component_power_supply_name 	"Джерело живлення"

#define dev_class_energy				"energy"
#define dev_class_energy_name   	    "Енергія"
#define dev_class_energy_state			"\"state_class\": \"total_increasing\",\n"
#define dev_class_energy_unit_of_measurement	"kWh"

#define dev_class_voltage				"voltage"
#define dev_class_voltage_name			"Напруга"
#define dev_class_voltage_unit_of_measurement	"V"

#define dev_class_power					"power"
#define dev_class_power_name			"Активна потужність"
#define dev_class_power_unit_of_measurement	"W"

#define dev_class_apparent_power		"apparent_power"
#define dev_class_apparent_power_name	"Повна потужність"
#define dev_class_apparent_power_unit_of_measurement	"VA"

#define dev_class_power_factor			"power_factor"
#define dev_class_power_factor_name	    "Коефіцієнт потужності"
#define dev_class_power_factor_unit_of_measurement "%"

#define dev_class_current				"current"
#define dev_class_current_name			"Струм"
#define dev_class_current_unit_of_measurement	"A"



static char mqtt_dev_identifier[MQTT_UNIC_IDENTIFIER_LENGTH] = {0};
static char	dev_conf_ip [INET6_ADDRSTRLEN]	= {0}; //IPv4 or IPv6
char home_assistant_prefix[] = {"homeassistant"};



int set_device_id(const uint8_t* id, unsigned const int id_len){

    if (id == NULL || id_len == 0) {
        return -EINVAL;
    }

    /* Нужно 2 символа на байт + 1 под '\0' */
    const size_t need = id_len * 2u + 1u;
    if (need > sizeof(mqtt_dev_identifier)) {
        return -ENOSPC;
    }

    // Проходим по каждому байту идентификатора
    for (unsigned int i = 0; i < id_len; i++) {
        // Конвертируем каждый байт в два символа и добавляем в строку
        sprintf(&mqtt_dev_identifier[i * 2], "%02X", id[i]);
    }
    return 0;
}

const char *get_device_id(void)
/* Возвращает указатель на постоянную строку с ID (или NULL, если не установлен). */
{
    /* k_mutex_lock(&dev_id_lock, K_FOREVER); */
    bool empty = (mqtt_dev_identifier[0] == '\0');
    /* k_mutex_unlock(&dev_id_lock); */
    return empty ? NULL : mqtt_dev_identifier;
}

// валидируем IPv4/IPv6 и только потом сохраняем */
int set_device_conf_ip(const char *ip_str)
{
    if (ip_str == NULL) {
        return -EINVAL;
    }

    /* Проверим, валиден ли IPv4 или IPv6 */
    struct in_addr  v4;
    struct in6_addr v6;

    int ok_v4 = inet_pton(AF_INET,  ip_str, &v4) == 1;
    int ok_v6 = inet_pton(AF_INET6, ip_str, &v6) == 1;

    if (!ok_v4 && !ok_v6) {
        return -EINVAL; /* не распознали адрес */
    }

    const int n = snprintf(dev_conf_ip, sizeof(dev_conf_ip), "%s", ip_str);
    if (n < 0) {
        return n;
    }
    if (n >= (int)sizeof(dev_conf_ip)) {
        return -ENOSPC;
    }
    return 0;
}

const char *get_device_conf_ip(void)
{
    return dev_conf_ip[0] ? dev_conf_ip : NULL;
}

int get_config_topik_string (char * buff, uint32_t buff_len, mqtt_ha_dev_type_t dev_type, uint8_t obj_number){

	strcpy(buff, "");
	switch (dev_type) {//%s/%s/%s_%s/%s%u/config
		case INPUT_SENSOR:
			snprintf(buff, buff_len, ha_universal_config_topik_template, home_assistant_prefix, component_binary_sensor, dev_system, mqtt_dev_identifier, component_input, obj_number);
			break;
		case OUTPUT_DEVICE:
			snprintf(buff, buff_len, ha_universal_config_topik_template, home_assistant_prefix, component_switch, dev_system, mqtt_dev_identifier, dev_class_switch, obj_number);
			break;
	    case VOLTAGE_DIAGNOSTIC_BATT_SENSOR:
			snprintf(buff, buff_len, ha_universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, mqtt_dev_identifier, component_battery, obj_number);
			break;
	    case VOLTAGE_DIAGNOSTIC_POW_SUPL_SENSOR:
	    	snprintf(buff, buff_len, ha_universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, mqtt_dev_identifier, component_power_supply, obj_number);
	    	break;
		case ENERGY_SENSOR:
			snprintf(buff, buff_len, ha_universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, mqtt_dev_identifier, dev_class_energy, obj_number);
			break;
		case VOLTAGE_SENSOR:
			snprintf(buff, buff_len, ha_universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, mqtt_dev_identifier, dev_class_voltage, obj_number);
			break;
		case POWER_SENSOR:
			snprintf(buff, buff_len, ha_universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, mqtt_dev_identifier, dev_class_power, obj_number);
			break;
		case APPARENT_POWER_SENSOR:
			snprintf(buff, buff_len, ha_universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, mqtt_dev_identifier, dev_class_apparent_power, obj_number);
			break;
		case POWER_FACTOR_SENSOR:
			snprintf(buff, buff_len, ha_universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, mqtt_dev_identifier, dev_class_power_factor, obj_number);
			break;
		case CURRENT_SENSOR:
			snprintf(buff, buff_len, ha_universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, mqtt_dev_identifier, dev_class_current, obj_number);
			break;

		default:
			return -1;
			break;
	}
	return 0;
}
