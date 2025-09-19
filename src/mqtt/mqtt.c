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

LOG_MODULE_REGISTER(mqtt);

K_THREAD_STACK_DEFINE(mqtt_task_stack, 2048);
static struct k_thread mqtt_task_thread_data;

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
        rv = dns_query("pool.ntp.org", 123,
                       family, SOCK_DGRAM, &addr, &addrlen);
        if (rv != 0) {
            LOG_ERR("Failed to lookup %s SNTP server (%d)", family_str, rv);
        }else {
            LOG_ERR("SNTP server: %s", inet_ntoa(net_sin(&addr)->sin_addr));
        }
        k_sleep(K_SECONDS(1));
    }

}

void app_mqtt_ha_client_init(void) {

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