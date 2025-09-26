#include <string.h>
#include <zephyr/settings/settings.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/dhcpv4.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/net/ethernet_mgmt.h>
#include <zephyr/logging/log.h>
#include <zephyr/version.h>

#include "web/http_server_init.h"
#include "mqtt/ha_mqtt.h"
#include "io/io.h"
#include "littlefs/littlefs_mount.h"


#include <zephyr/logging/log_backend.h>
#include <zephyr/logging/log_backend_net.h>
#include <zephyr/logging/log_ctrl.h>

#include "test_functions.h"


LOG_MODULE_REGISTER(main_app, LOG_LEVEL_INF);

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

/* Получаем numeric ID разделов из DTS по алиасам узлов fixed-partitions */
#define LFS_PART_ID  FIXED_PARTITION_ID(storage_lfs_partition)
#define NVS_PART_ID  FIXED_PARTITION_ID(storage_nvs_partition)



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

	const struct log_backend *backend = log_backend_net_get();
	if (!log_backend_is_active(backend)) {

		/* Specifying an address by calling this function will
		 * override the value given to LOG_BACKEND_NET_SERVER.
		   It can also be called at any other time after the backend
		   is started. The net context will be released and
		   restarted with the newly specified address.
		 */
		log_backend_net_set_addr("192.168.88.182:514");
		log_backend_init(backend);
		log_backend_enable(backend, backend->cb->ctx, LOG_LEVEL_DBG);
		log_backend_net_start();
	}

	app_http_server_init();
	app_mqtt_ha_client_init();


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

/* Универсальная функция стирания раздела целиком */
static int erase_partition_by_id(uint8_t part_id)
{
	const struct flash_area *fa;
	int rc = flash_area_open(part_id, &fa);
	if (rc) {
		LOG_ERR("flash_area_open(%u) failed: %d", part_id, rc);
		return rc;
	}

	LOG_INF("Erasing partition id=%u, offset=0x%lx, size=0x%lx",
			(unsigned)part_id, (unsigned long)fa->fa_off, (unsigned long)fa->fa_size);

	/* Важно: перед стиранием убедитесь, что ФС на этом разделе размонтирована,
	   и никакие задачи его не используют. */

	rc = flash_area_erase(fa, 0, fa->fa_size);  /* стираем ВСЮ область */
	flash_area_close(fa);

	if (rc) {
		LOG_ERR("flash_area_erase failed: %d", rc);
		return rc;
	}

	LOG_INF("Erase OK");
	return 0;
}

int main(void)
{
	int ret;

	LOG_INF("Start main app");

	//int usb_ret = usb_enable(NULL);
	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("Error: LED device is not ready\n");
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Error %d: failed to configure LED\n", ret);
		return;
	}

	const struct device *flash = DEVICE_DT_GET(FLASH_NODE);

	if (!device_is_ready(flash)) {
		LOG_ERR("SPI_FLASH not ready (probe failed or disabled)");
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

	//(void)erase_partition_by_id(LFS_PART_ID);
	//(void)erase_partition_by_id(NVS_PART_ID);

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

	const struct flash_parameters *params = flash_get_parameters(flash_dev);
	if (params) {
		printk("Erase value: 0x%02x\n", params->erase_value);
		printk("Write block size: %u\n", params->write_block_size);
	}

	/* Подписка на событие получения IPv4 адреса */
	net_mgmt_init_event_callback(&cb, ipv4_addr_add_handler, NET_EVENT_IPV4_ADDR_ADD);
	net_mgmt_add_event_callback(&cb);

	/* Включаем интерфейс по умолчанию (w5500) */
	struct net_if *iface = net_if_get_default();
	uint8_t mac[6];
	make_mac_from_uid(mac);

	(void)settings_subsys_init();
	(void)settings_load();
	(void)fs_service_init();
	io_init();


	if (iface) {
		net_if_up(iface); /* DHCP стартует автоматически при CONFIG_NET_DHCPV4=y */
		(void)net_dhcpv4_start(iface);    // ЯВНО запустить DHCP
		LOG_INF("Network interface up");
	}



	int answ = 0;
	answ = fs_mkdir("/lfs/test1");
	answ = fs_mkdir("/lfs/test1/test2");
	answ = fs_mkdir("/lfs/test1/test2/test3");
	//fs_delete_tree("/lfs/test1");

	fs_mkdir("/lfs/www");

	create_index_html();

	LOG_INF("answ = %d", answ);

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			printk("Error %d: failed to toggle LED\n", ret);
			return;
		}

		k_msleep(SLEEP_TIME_MS);
	}
}