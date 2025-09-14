#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/device.h>
// #include <zephyr/logging/log.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/dhcpv4.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/net/ethernet_mgmt.h>

//#include "io.h"
//#include "fs_mount.h"
//#include "nvs_mount.h"
//#include "app/web.h"
//#include "topics.h"
//#include "app/home_assistant_mqtt.h"

#include <string.h>

#include <zephyr/settings/settings.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#define FLASH_NODE DT_NODELABEL(spi_flash)

#if !DT_NODE_HAS_STATUS(FLASH_NODE, okay)
#error "DT node 'spi_flash' not found or not okay"
#endif

#define STM32_UID_BASE  0x1FFF7A10U

#define STORAGE_PARTITION	storage_partition
#define STORAGE_PARTITION_ID	FIXED_PARTITION_ID(STORAGE_PARTITION)

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static void dump_hex(const uint8_t *buf, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		printk("%02X%s", buf[i], ((i + 1) % 16) ? " " : "\n");
	}
	if (len % 16) {
		printk("\n");
	}
}

static struct net_mgmt_event_callback cb;

static void ipv4_addr_add_handler(struct net_mgmt_event_callback *cb,
								  uint32_t mgmt_event, struct net_if *iface)
{
	//k_msleep(SLEEP_TIME_MS * 10);
	//web_start();
	//home_assistant_mqtt_start();
	if (mgmt_event != NET_EVENT_IPV4_ADDR_ADD) {
		return;
	}

	char addr_str[NET_IPV4_ADDR_LEN];
	struct net_if_addr *ifaddr = net_if_ipv4_get_global_addr(iface, NET_ADDR_DHCP);
	if (!ifaddr) {
		ifaddr = net_if_ipv4_get_global_addr(iface, NET_ADDR_MANUAL);
	}
	if (ifaddr) {
		net_addr_ntop(AF_INET, &ifaddr->address.in_addr, addr_str, sizeof(addr_str));
		// LOG_INF("IPv4 address acquired: %s", addr_str);
	} else {
		// LOG_INF("IPv4 address acquired (type unknown)");
	}
}

static void make_mac_from_uid(uint8_t mac[6])
{
	const uint32_t *uid = (const uint32_t *)STM32_UID_BASE;
	uint32_t a = uid[0], b = uid[1], c = uid[2];

	// Простой mix (можно заменить на xxhash/CRC32, если включено)
	uint32_t h = a ^ (b << 11) ^ (c << 22) ^ (a >> 7) ^ (b >> 3);
	// Байт 0: local admin (bit1=1), unicast (bit0=0); выберите свой OUI при необходимости
	mac[0] = 0x02;           // LAA + unicast
	mac[1] = 0x00;
	mac[2] = 0x5E;
	mac[3] = (uint8_t)(h >> 16);
	mac[4] = (uint8_t)(h >> 8);
	mac[5] = (uint8_t)(h);
}



int main(void)
{
	int ret;

	int usb_ret = usb_enable(NULL);
	if (!gpio_is_ready_dt(&led)) {
		printk("Error: LED device is not ready\n");
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error %d: failed to configure LED\n", ret);
		return;
	}

	const struct device *flash = DEVICE_DT_GET(FLASH_NODE);

	if (!device_is_ready(flash)) {
		// LOG_ERR("SPI_FLASH not ready (probe failed or disabled)");
		return;
	}

	const struct flash_parameters *p = flash_get_parameters(flash);
	if (p) {
		printk("erase_value=0x%02x, write_block=%u",
				p->erase_value, p->write_block_size);
	}

	//const struct device *flash_dev = DEVICE_DT_GET(FLASH_NODE);

	const struct device *flash_dev = DEVICE_DT_GET(DT_NODELABEL(spi_flash));
	if (!device_is_ready(flash_dev)) {
		printk("Flash device not ready\n");
		return;
	}

	// struct flash_pages_info info;
	// if (flash_get_page_info_by_offs(flash_dev, 0, &info) == 0) {
	// 	size_t page_size = info.size;
	// 	size_t page_count = flash_get_page_count(flash_dev);
	// 	size_t total_size = page_size * page_count;
	//
	// 	printk("Flash total size: %u bytes (%u KiB)\n",
	// 		   (unsigned int)total_size,
	// 		   (unsigned int)(total_size / 1024));
	// 	printk("Page size: %u bytes\n", (unsigned int)page_size);
	// 	printk("Page count: %u\n", (unsigned int)page_count);
	// } else {
	// 	printk("Failed to get flash info\n");
	// }
	//
	// const struct flash_parameters *params = flash_get_parameters(flash_dev);
	// if (params) {
	// 	printk("Erase value: 0x%02x\n", params->erase_value);
	// 	printk("Write block size: %u\n", params->write_block_size);
	// }

	/* Подписка на событие получения IPv4 адреса */
	net_mgmt_init_event_callback(&cb, ipv4_addr_add_handler, NET_EVENT_IPV4_ADDR_ADD);
	net_mgmt_add_event_callback(&cb);

	/* Включаем интерфейс по умолчанию (w5500) */
	struct net_if *iface = net_if_get_default();
	uint8_t mac[6];
	make_mac_from_uid(mac);

	//fs_subsystem();
	//nvs_service_init();
	//web_start();
	//settings_subsys_init();
	//settings_load();

	//settings_save_one("app/mode", "test", sizeof("test")+1);

	if (iface) {
		net_if_up(iface); /* DHCP стартует автоматически при CONFIG_NET_DHCPV4=y */
		(void)net_dhcpv4_start(iface);    // ЯВНО запустить DHCP
		//LOG_INF("Network interface up");

	}

	//io_start();

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			printk("Error %d: failed to toggle LED\n", ret);
			return;
		}

		k_msleep(SLEEP_TIME_MS);
	}
}