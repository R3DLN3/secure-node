#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>

LOG_MODULE_REGISTER(ssn, LOG_LEVEL_INF);

static uint32_t heartbeat_count;
static bool notify_enabled;

/* 128-bit UUIDs for our custom service + heartbeat characteristic */
#define BT_UUID_SSN_SERVICE_VAL   BT_UUID_128_ENCODE(0x12345678,0x1234,0x5678,0x1234,0x56789abcdef0)
#define BT_UUID_SSN_HEARTBEAT_VAL BT_UUID_128_ENCODE(0x12345678,0x1234,0x5678,0x1234,0x56789abcdef1)

static struct bt_uuid_128 ssn_service_uuid   = BT_UUID_INIT_128(BT_UUID_SSN_SERVICE_VAL);
static struct bt_uuid_128 ssn_heartbeat_uuid = BT_UUID_INIT_128(BT_UUID_SSN_HEARTBEAT_VAL);

static void ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	notify_enabled = (value == BT_GATT_CCC_NOTIFY);
	LOG_INF("CCC changed: notify %s", notify_enabled ? "ON" : "OFF");
}

static ssize_t read_heartbeat(struct bt_conn *conn,
			      const struct bt_gatt_attr *attr,
			      void *buf, uint16_t len, uint16_t offset)
{
	uint32_t v_le = sys_cpu_to_le32(heartbeat_count);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &v_le, sizeof(v_le));
}

BT_GATT_SERVICE_DEFINE(ssn_svc,
	BT_GATT_PRIMARY_SERVICE(&ssn_service_uuid),
	BT_GATT_CHARACTERISTIC(&ssn_heartbeat_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ,
			       read_heartbeat, NULL, NULL),
	BT_GATT_CCC(ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);

/* Keep advertising data small */
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	/* Optional: show “Battery Service” in scanner UI */
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0F, 0x18),
	/* Also advertise our custom service UUID so it shows under Advertised Services */
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_SSN_SERVICE_VAL),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE,
		CONFIG_BT_DEVICE_NAME,
		(sizeof(CONFIG_BT_DEVICE_NAME) - 1)),
};

static int start_advertising(void)
{
	int err;

#if defined(BT_LE_ADV_CONN_FAST_1)
	const struct bt_le_adv_param *param = BT_LE_ADV_CONN_FAST_1;
	LOG_INF("Starting CONNECTABLE advertising (FAST_1)...");
#elif defined(BT_LE_ADV_CONN_FAST_2)
	const struct bt_le_adv_param *param = BT_LE_ADV_CONN_FAST_2;
	LOG_INF("Starting CONNECTABLE advertising (FAST_2)...");
#else
	const struct bt_le_adv_param *param = BT_LE_ADV_NCONN;
	LOG_WRN("No connectable ADV macro found; falling back to NON-connectable advertising!");
#endif

	err = bt_le_adv_start(param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("bt_le_adv_start failed (err %d)", err);
		return err;
	}

	LOG_INF("Advertising started");
	return 0;
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_WRN("Connection failed (err 0x%02x)", err);
		return;
	}
	LOG_INF("Connected");
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	notify_enabled = false;
	LOG_INF("Disconnected (reason 0x%02x) - restarting advertising", reason);

	(void)bt_le_adv_stop();
	(void)start_advertising();
}

BT_CONN_CB_DEFINE(conn_cb) = {
	.connected = connected,
	.disconnected = disconnected,
};

void main(void)
{
	int err = bt_enable(NULL);
	if (err) {
		LOG_ERR("bt_enable failed (err %d)", err);
		return;
	}

	printk("[UART] BOOT: secure_sensor_node is running!\n");
	printk("[UART] Device name = '%s'\n", CONFIG_BT_DEVICE_NAME);

	err = start_advertising();
	if (err) {
		return;
	}

	while (1) {
		printk("[UART] Heartbeat %u\n", heartbeat_count);

		if (notify_enabled) {
			uint32_t v_le = sys_cpu_to_le32(heartbeat_count);
			/* ssn_svc.attrs[2] is the characteristic VALUE attribute */
			bt_gatt_notify(NULL, &ssn_svc.attrs[2], &v_le, sizeof(v_le));
		}

		heartbeat_count++;
		k_sleep(K_SECONDS(1));
	}
}
