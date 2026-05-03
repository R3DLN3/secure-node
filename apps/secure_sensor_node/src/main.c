#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

#include <string.h>
#include <stdint.h>

LOG_MODULE_REGISTER(ssn, LOG_LEVEL_INF);

static uint32_t heartbeat_count;
static uint32_t tamper_count;
static bool notify_enabled;
static int64_t last_press_time;

static const struct device *display_dev;

/* NVS / flash storage */
static struct nvs_fs fs;

/* NVS IDs */
#define NVS_ID_TAMPER_COUNT 1

/* 128-bit UUIDs */
#define BT_UUID_SSN_SERVICE_VAL   BT_UUID_128_ENCODE(0x12345678,0x1234,0x5678,0x1234,0x56789abcdef0)
#define BT_UUID_SSN_HEARTBEAT_VAL BT_UUID_128_ENCODE(0x12345678,0x1234,0x5678,0x1234,0x56789abcdef1)

static struct bt_uuid_128 ssn_service_uuid   = BT_UUID_INIT_128(BT_UUID_SSN_SERVICE_VAL);
static struct bt_uuid_128 ssn_heartbeat_uuid = BT_UUID_INIT_128(BT_UUID_SSN_HEARTBEAT_VAL);

/* Button */
#define SW0_NODE DT_ALIAS(sw0)

#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "sw0 alias is not defined in the devicetree"
#endif

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static struct gpio_callback button_cb_data;

static void ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	notify_enabled = (value == BT_GATT_CCC_NOTIFY);
}

static ssize_t read_heartbeat(struct bt_conn *conn,
			      const struct bt_gatt_attr *attr,
			      void *buf,
			      uint16_t len,
			      uint16_t offset)
{
	uint32_t v_le = sys_cpu_to_le32(heartbeat_count);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &v_le, sizeof(v_le));
}

BT_GATT_SERVICE_DEFINE(ssn_svc,
	BT_GATT_PRIMARY_SERVICE(&ssn_service_uuid),
	BT_GATT_CHARACTERISTIC(&ssn_heartbeat_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ,
			       read_heartbeat,
			       NULL,
			       NULL),
	BT_GATT_CCC(ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_SSN_SERVICE_VAL),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE,
		CONFIG_BT_DEVICE_NAME,
		sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static int start_advertising(void)
{
	return bt_le_adv_start(BT_LE_ADV_CONN_FAST_1,
			       ad, ARRAY_SIZE(ad),
			       sd, ARRAY_SIZE(sd));
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	ARG_UNUSED(conn);

	if (!err) {
		LOG_INF("Connected");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	ARG_UNUSED(conn);
	ARG_UNUSED(reason);

	notify_enabled = false;
	start_advertising();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static void oled_update(uint32_t hb, uint32_t tamper)
{
	ARG_UNUSED(hb);
	ARG_UNUSED(tamper);

	if (!device_is_ready(display_dev)) {
		return;
	}

	static uint8_t buf[128 * 4];
	memset(buf, 0x00, sizeof(buf));

	struct display_buffer_descriptor desc = {
		.buf_size = sizeof(buf),
		.width = 128,
		.height = 32,
		.pitch = 128,
	};

	int err = display_write(display_dev, 0, 0, &desc, buf);
	if (err) {
		LOG_ERR("display_write failed (%d)", err);
	}
}

static int storage_init(void)
{
	int err;
	const struct flash_area *fa;

	err = flash_area_open(FIXED_PARTITION_ID(storage_partition), &fa);
	if (err) {
		LOG_ERR("flash_area_open failed (%d)", err);
		return err;
	}

	fs.flash_device = fa->fa_dev;
	fs.offset = fa->fa_off;
	fs.sector_size = 4096;
	fs.sector_count = 3;

	err = nvs_mount(&fs);
	if (err) {
		LOG_ERR("nvs_mount failed (%d)", err);
		return err;
	}

	LOG_INF("NVS mounted");
	return 0;
}

static void storage_load_tamper_count(void)
{
	int rc;
	uint32_t stored_count = 0;

	rc = nvs_read(&fs, NVS_ID_TAMPER_COUNT, &stored_count, sizeof(stored_count));
	if (rc > 0) {
		tamper_count = stored_count;
		LOG_INF("Loaded tamper_count=%u from flash", tamper_count);
		printk("[UART] Loaded tamper_count=%u from flash\n", tamper_count);
	} else {
		tamper_count = 0;
		LOG_INF("No saved tamper_count found, starting at 0");
		printk("[UART] No saved tamper_count found, starting at 0\n");
	}
}

static void storage_save_tamper_count(void)
{
	int rc;

	rc = nvs_write(&fs, NVS_ID_TAMPER_COUNT, &tamper_count, sizeof(tamper_count));
	if (rc < 0) {
		LOG_ERR("Failed to save tamper_count (%d)", rc);
		printk("[UART] Failed to save tamper_count (%d)\n", rc);
		return;
	}

	LOG_INF("Saved tamper_count=%u to flash", tamper_count);
	printk("[UART] Saved tamper_count=%u to flash\n", tamper_count);
}

static void button_pressed(const struct device *dev,
			   struct gpio_callback *cb,
			   uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	int64_t now = k_uptime_get();

	if (now - last_press_time < 200) {
		return;
	}

	last_press_time = now;
	tamper_count++;

	printk("[UART] Tamper event! count=%u\n", tamper_count);

	storage_save_tamper_count();
}

static int button_init(void)
{
	int err;

	if (!gpio_is_ready_dt(&button)) {
		return -ENODEV;
	}

	err = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (err) {
		return err;
	}

	err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (err) {
		return err;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	return 0;
}

void main(void)
{
	int err;

	printk("[UART] BOOT: secure_sensor_node\n");

	display_dev = DEVICE_DT_GET_ANY(solomon_ssd1306fb);
	if (device_is_ready(display_dev)) {
		display_blanking_off(display_dev);
	}

	err = storage_init();
	if (err) {
		printk("[UART] Storage init failed\n");
	} else {
		storage_load_tamper_count();
	}

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed\n");
		return;
	}

	err = start_advertising();
	if (err) {
		printk("Advertising start failed\n");
		return;
	}

	err = button_init();
	if (err) {
		printk("Button init failed\n");
	}

	while (1) {
		printk("[UART] Heartbeat %u | Tamper %u\n",
		       heartbeat_count,
		       tamper_count);

		oled_update(heartbeat_count, tamper_count);

		if (notify_enabled) {
			uint32_t v_le = sys_cpu_to_le32(heartbeat_count);

			bt_gatt_notify(NULL,
				       &ssn_svc.attrs[2],
				       &v_le,
				       sizeof(v_le));
		}

		heartbeat_count++;
		k_sleep(K_SECONDS(1));
	}
}