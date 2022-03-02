#include <zephyr.h>
#include <init.h>

#include <logging/log.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/gap.h>
#include <settings/settings.h>

//#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(BT_APP, LOG_LEVEL_INF);

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param);
static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout);

static struct k_work advertise_work;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
};

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
	.le_param_req = le_param_req,
	.le_param_updated = le_param_updated,
};

static int settings_runtime_load(void)
{
#if defined(CONFIG_BT_DIS_SETTINGS)
#if defined(CONFIG_BT_DIS_MODEL)
	settings_runtime_set("bt/dis/model",
				 CONFIG_BT_DIS_MODEL,
				 sizeof(CONFIG_BT_DIS_MODEL));
#endif
#if defined(CONFIG_BT_DIS_MANUF)
	settings_runtime_set("bt/dis/manuf",
				 CONFIG_BT_DIS_MANUF,
				 sizeof(CONFIG_BT_DIS_MANUF));
#endif
#if defined(CONFIG_BT_DIS_SERIAL_NUMBER)
	settings_runtime_set("bt/dis/serial",
				 CONFIG_BT_DIS_SERIAL_NUMBER_STR,
				 sizeof(CONFIG_BT_DIS_SERIAL_NUMBER_STR));
#endif
#if defined(CONFIG_BT_DIS_SW_REV)
	settings_runtime_set("bt/dis/sw",
				 CONFIG_BT_DIS_SW_REV_STR,
				 sizeof(CONFIG_BT_DIS_SW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_FW_REV)
	settings_runtime_set("bt/dis/fw",
				 CONFIG_BT_DIS_FW_REV_STR,
				 sizeof(CONFIG_BT_DIS_FW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_HW_REV)
	settings_runtime_set("bt/dis/hw",
				 CONFIG_BT_DIS_HW_REV_STR,
				 sizeof(CONFIG_BT_DIS_HW_REV_STR));
#endif
#endif
	return 0;
}

static void advertise(struct k_work *work)
{
	int rc;

	bt_le_adv_stop();

	rc = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (rc) {
		LOG_ERR("Advertising failed to start (rc %d)", rc);
		return;
	}

	LOG_INF("Advertising successfully started");
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		return;
	}

	LOG_INF("connected");
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("disconnected (reason: %u)", reason);
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	return true;
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
}

int app_bt_init(void)
{
	int err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return err;
	}
#if 0
	err = settings_load();
	if (err) {
		LOG_ERR("Settings load failed (err %d)", err);
	}

	err = settings_runtime_load();
	if (err) {
		LOG_ERR("Settings runtime load failed (err %d)", err);
	}
#endif
	k_work_init(&advertise_work, advertise);
	k_work_submit(&advertise_work);
	LOG_INF("Bluetooth initialized");

	return err;
}
