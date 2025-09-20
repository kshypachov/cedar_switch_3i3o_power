//
// Created by Kirill Shypachov on 14.09.2025.
//
#include "mqtt.h"
#include "../zbus_topics.h"
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_service.h>
#include <zephyr/net/sntp.h>
#include <arpa/inet.h>
#include <zephyr/net/mqtt.h>
#include "../settings_topics.h"
#include <zephyr/settings/settings.h>
#include "mqtt_callback.h"

LOG_MODULE_REGISTER(mqtt);

K_THREAD_STACK_DEFINE(mqtt_task_stack, 2048);
static struct k_thread mqtt_task_thread_data;

mqtt_settings_t mqtt_sett = {0};
/* The mqtt client struct */
static struct mqtt_client client_ctx;

/* MQTT Broker details. */
static struct sockaddr_storage broker;

#define APP_MQTT_BUFFER_SIZE	1024

/* Buffers for MQTT client. */
static uint8_t rx_buffer[APP_MQTT_BUFFER_SIZE];
static uint8_t tx_buffer[APP_MQTT_BUFFER_SIZE];

#define MQTT_CLIENTID		"zephyr_publisher"



static int dns_query(const char *host, uint16_t port, int family, int socktype, struct sockaddr *addr,
              socklen_t *addrlen)
{
    struct addrinfo hints = {
        .ai_family = family,
        .ai_socktype = socktype,
    };
    struct addrinfo *res = NULL;
    char addr_str[INET6_ADDRSTRLEN] = {0};
    int rv;

    /* Perform DNS query */
    rv = getaddrinfo(host, NULL, &hints, &res);
    if (rv < 0) {
        LOG_ERR("getaddrinfo failed (%d, errno %d)", rv, errno);
        return rv;
    }
    /* Store the first result */
    *addr = *res->ai_addr;
    *addrlen = res->ai_addrlen;
    /* Free the allocated memory */
    freeaddrinfo(res);
    /* Store the port */
    net_sin(addr)->sin_port = htons(port);
    /* Print the found address */
    inet_ntop(addr->sa_family, &net_sin(addr)->sin_addr, addr_str, sizeof(addr_str));
    LOG_INF("%s -> %s", host, addr_str);
    return 0;
}

static void client_init(struct mqtt_client *client, struct sockaddr *addr)
{
	mqtt_client_init(client);

	//broker_init();
	//broker = addr;
	/* MQTT client configuration */
	//client->broker = &broker;
	client->broker = addr;
	client->evt_cb = mqtt_evt_handler;
	client->client_id.utf8 = (uint8_t *)MQTT_CLIENTID;
	client->client_id.size = strlen(MQTT_CLIENTID);
	client->password = NULL;
	client->user_name = NULL;
#if defined(CONFIG_MQTT_VERSION_5_0)
	client->protocol_version = MQTT_VERSION_5_0;
#else
	client->protocol_version = MQTT_VERSION_3_1_1;
#endif

	/* MQTT buffers configuration */
	client->rx_buf = rx_buffer;
	client->rx_buf_size = sizeof(rx_buffer);
	client->tx_buf = tx_buffer;
	client->tx_buf_size = sizeof(tx_buffer);

	/* MQTT transport configuration */
#if defined(CONFIG_MQTT_LIB_TLS)
#if defined(CONFIG_MQTT_LIB_WEBSOCKET)
	client->transport.type = MQTT_TRANSPORT_SECURE_WEBSOCKET;
#else
	client->transport.type = MQTT_TRANSPORT_SECURE;
#endif

	struct mqtt_sec_config *tls_config = &client->transport.tls.config;

	tls_config->peer_verify = TLS_PEER_VERIFY_REQUIRED;
	tls_config->cipher_list = NULL;
	tls_config->sec_tag_list = m_sec_tags;
	tls_config->sec_tag_count = ARRAY_SIZE(m_sec_tags);
#if defined(MBEDTLS_X509_CRT_PARSE_C) || defined(CONFIG_NET_SOCKETS_OFFLOAD)
	tls_config->hostname = TLS_SNI_HOSTNAME;
#else
	tls_config->hostname = NULL;
#endif

#else
#if defined(CONFIG_MQTT_LIB_WEBSOCKET)
	client->transport.type = MQTT_TRANSPORT_NON_SECURE_WEBSOCKET;
#else
	client->transport.type = MQTT_TRANSPORT_NON_SECURE;
#endif
#endif

#if defined(CONFIG_MQTT_LIB_WEBSOCKET)
	client->transport.websocket.config.host = SERVER_ADDR;
	client->transport.websocket.config.url = "/mqtt";
	client->transport.websocket.config.tmp_buf = temp_ws_rx_buf;
	client->transport.websocket.config.tmp_buf_len =
						sizeof(temp_ws_rx_buf);
	client->transport.websocket.timeout = 5 * MSEC_PER_SEC;
#endif

#if defined(CONFIG_SOCKS)
	mqtt_client_set_proxy(client, &socks5_proxy,
			      socks5_proxy.sa_family == AF_INET ?
			      sizeof(struct sockaddr_in) :
			      sizeof(struct sockaddr_in6));
#endif
}

static void mqtt_task(void *a, void *b, void *c) {

    ARG_UNUSED(a);
    ARG_UNUSED(b);
    ARG_UNUSED(c);

    int family = AF_INET;
    char *family_str = family == AF_INET ? "IPv4" : "IPv6";
    struct sntp_time s_time;
    struct sntp_ctx ctx;
    struct sockaddr addr;
    socklen_t addrlen;
    int rv;


    while (1) {
        /* Get SNTP server */
        rv = dns_query("pool.ntp.org", 8883, family, SOCK_DGRAM, &addr, &addrlen);
        if (rv != 0) {
            LOG_ERR("Failed to lookup %s MQTT server (%d)", family_str, rv);
        }else {

        	client_init(&client_ctx, &addr);

            LOG_INF("MQTT server: %s", inet_ntoa(net_sin(&addr)->sin_addr));

        	int rc = mqtt_connect(&client_ctx);
        	if (rc != 0) {
        		LOG_INF("mqtt_connect", rc);
        		k_sleep(K_SECONDS(1));

        	} else {
        		while (1) {
        			k_sleep(K_SECONDS(1));
        		}
        	}

        }
        k_sleep(K_SECONDS(1));
    }

}

void app_mqtt_ha_client_init(void) {

    settings_load_one(mqtt_enabled_settings, &mqtt_sett.enabled, sizeof(mqtt_sett.enabled));
    settings_load_one(mqtt_host_settings, mqtt_sett.host, sizeof(mqtt_sett.host));
	settings_load_one(mqtt_port_settings, &mqtt_sett.port, sizeof(mqtt_sett.port));
    settings_load_one(mqtt_user_settings, mqtt_sett.user, sizeof(mqtt_sett.user));
    settings_load_one(mqtt_pass_settings, mqtt_sett.pass, sizeof(mqtt_sett.enabled));

    if (mqtt_sett.enabled) {

        k_tid_t tid = k_thread_create(&mqtt_task_thread_data,
                                  mqtt_task_stack,
                                  K_THREAD_STACK_SIZEOF(mqtt_task_stack),
                                  mqtt_task,
                                  NULL, NULL, NULL,
                                  4,
                                  0,
                                  K_NO_WAIT);

        k_thread_name_set(tid, "mqtt_task");
    }
}