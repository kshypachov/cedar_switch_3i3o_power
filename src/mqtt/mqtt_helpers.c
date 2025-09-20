//
// Created by Kirill Shypachov on 20.09.2025.
//

#include "mqtt_helpers.h"

// static void client_init(struct mqtt_client *client)
// {
// 	mqtt_client_init(client);
//
// 	broker_init();
//
// 	/* MQTT client configuration */
// 	client->broker = &broker;
// 	client->evt_cb = mqtt_evt_handler;
// 	client->client_id.utf8 = (uint8_t *)MQTT_CLIENTID;
// 	client->client_id.size = strlen(MQTT_CLIENTID);
// 	client->password = NULL;
// 	client->user_name = NULL;
// #if defined(CONFIG_MQTT_VERSION_5_0)
// 	client->protocol_version = MQTT_VERSION_5_0;
// #else
// 	client->protocol_version = MQTT_VERSION_3_1_1;
// #endif
//
// 	/* MQTT buffers configuration */
// 	client->rx_buf = rx_buffer;
// 	client->rx_buf_size = sizeof(rx_buffer);
// 	client->tx_buf = tx_buffer;
// 	client->tx_buf_size = sizeof(tx_buffer);
//
// 	/* MQTT transport configuration */
// #if defined(CONFIG_MQTT_LIB_TLS)
// #if defined(CONFIG_MQTT_LIB_WEBSOCKET)
// 	client->transport.type = MQTT_TRANSPORT_SECURE_WEBSOCKET;
// #else
// 	client->transport.type = MQTT_TRANSPORT_SECURE;
// #endif
//
// 	struct mqtt_sec_config *tls_config = &client->transport.tls.config;
//
// 	tls_config->peer_verify = TLS_PEER_VERIFY_REQUIRED;
// 	tls_config->cipher_list = NULL;
// 	tls_config->sec_tag_list = m_sec_tags;
// 	tls_config->sec_tag_count = ARRAY_SIZE(m_sec_tags);
// #if defined(MBEDTLS_X509_CRT_PARSE_C) || defined(CONFIG_NET_SOCKETS_OFFLOAD)
// 	tls_config->hostname = TLS_SNI_HOSTNAME;
// #else
// 	tls_config->hostname = NULL;
// #endif
//
// #else
// #if defined(CONFIG_MQTT_LIB_WEBSOCKET)
// 	client->transport.type = MQTT_TRANSPORT_NON_SECURE_WEBSOCKET;
// #else
// 	client->transport.type = MQTT_TRANSPORT_NON_SECURE;
// #endif
// #endif
//
// #if defined(CONFIG_MQTT_LIB_WEBSOCKET)
// 	client->transport.websocket.config.host = SERVER_ADDR;
// 	client->transport.websocket.config.url = "/mqtt";
// 	client->transport.websocket.config.tmp_buf = temp_ws_rx_buf;
// 	client->transport.websocket.config.tmp_buf_len =
// 						sizeof(temp_ws_rx_buf);
// 	client->transport.websocket.timeout = 5 * MSEC_PER_SEC;
// #endif
//
// #if defined(CONFIG_SOCKS)
// 	mqtt_client_set_proxy(client, &socks5_proxy,
// 			      socks5_proxy.sa_family == AF_INET ?
// 			      sizeof(struct sockaddr_in) :
// 			      sizeof(struct sockaddr_in6));
// #endif
// }