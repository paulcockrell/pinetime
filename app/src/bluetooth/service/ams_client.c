#include <bluetooth/gatt.h>

#include "ams_c.h"

#define LOG_MODULE_NAME bt_ams_client
#include "common/log.h"

static uint8_t entity_update_notify(struct bt_conn* conn,
                                 struct bt_gatt_subscribe_params* params,
                                 const void* data, uint16_t length)
{
    const uint8_t* bytes = data;
    printk("EntityID: %u, AttributeID: %u, Flags: %u, Value: ", bytes[0],
           bytes[1], bytes[2]);
    for (uint16_t i = 3; i < length; ++i) {
        printk("%c", bytes[i]);
    }
    printk("\n");
    return BT_GATT_ITER_CONTINUE;
}

static void entity_update_write_response(struct bt_conn* conn, uint8_t err,
                                         struct bt_gatt_write_params* params)
{

}

int ams_client_entity_write(struct bt_ams *inst,
			    enum ams_entity_id entity_id,
			    enum ams_player_attribute_id attr_id)
{
	inst->cli.entity_update_command[0] = entity_id;
	inst->cli.entity_update_command[1] = attr_id;
	inst->cli.write_params.data = inst->cli.entity_update_command;
	inst->cli.write_params.length = 2;
	inst->cli.write_params.offset = 0;
	inst->cli.write_params.func = entity_update_write_response;
	inst->cli.write_params.handle = inst->cli.entity_write_handle;

	return bt_gatt_write(inst->cli.conn, &inst->cli.write_params);
}

static uint8_t ams_discover_func(struct bt_conn *conn, const struct bt_gatt_attr *attr,
				 struct bt_gatt_discover_params *params)
{
	struct bt_ams_client *client_inst = CONTAINER_OF(params,
							 struct bt_ams_client,
							 discover_params);

	struct bt_ams *inst = CONTAINER_OF(client_inst, struct bt_ams, cli);

	if (params->type == BT_GATT_DISCOVER_CHARACTERISTIC) {
		struct bt_gatt_subscribe_params *sub_params = NULL;
		BT_DBG("Discovered attribute - uuid: %s, handle: %u\n", bt_uuid_str(params->uuid), attr->handle);
		if (!bt_uuid_cmp(params->uuid, BT_UUID_AMS_ENTITY_UPDATE)) {
			BT_DBG("AMS entity update");
			inst->cli.entity_write_handle = attr->handle + 1;;
			inst->cli.entity_subscribe_handle = attr->handle + 1;
		} else if (!bt_uuid_cmp(params->uuid, BT_UUID_AMS_ENTITY_ATTR)) {
			BT_DBG("AMS entity attr");
		}
		if (sub_params) {
//bt_gatt_subscribe
		}
	}

	return BT_GATT_ITER_CONTINUE;
}

static uint8_t primary_discover_func(struct bt_conn *conn,
				     const struct bt_gatt_attr *attr,
				     struct bt_gatt_discover_params *params)
{
	struct bt_ams_client *client_inst = CONTAINER_OF(params,
							 struct bt_ams_client,
							 discover_params);

	struct bt_ams *inst = CONTAINER_OF(client_inst, struct bt_ams, cli);

	if (params->type == BT_GATT_DISCOVER_PRIMARY) {
		int err = 0;
		struct bt_gatt_service_val* gatt_service = attr->user_data;
		BT_DBG("Primary discover complete");

		inst->cli.start_handle = attr->handle + 1;
		inst->cli.end_handle = gatt_service->end_handle;

		inst->cli.discover_params.uuid = NULL;
		inst->cli.discover_params.start_handle = inst->cli.start_handle;
		inst->cli.discover_params.end_handle = inst->cli.end_handle;
		inst->cli.discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
		inst->cli.discover_params.func = ams_discover_func;

		err = bt_gatt_discover(conn, &inst->cli.discover_params);
		if (err) {
			BT_DBG("Discover failed (err %d)", err);
		}
		return BT_GATT_ITER_STOP;
	}

	return BT_GATT_ITER_CONTINUE;
}

int bt_ams_discover(struct bt_conn *conn, struct bt_ams *inst)
{
	int err = 0;

	memcpy(&inst->cli.uuid, BT_UUID_AMS, sizeof(inst->cli.uuid));

	inst->cli.conn = conn;
	inst->cli.discover_params.func = primary_discover_func;
	inst->cli.discover_params.uuid = &inst->cli.uuid.uuid;
	inst->cli.discover_params.type = BT_GATT_DISCOVER_PRIMARY;
	inst->cli.discover_params.start_handle = BT_ATT_FIRST_ATTTRIBUTE_HANDLE;
	inst->cli.discover_params.end_handle = BT_ATT_LAST_ATTTRIBUTE_HANDLE;

	err = bt_gatt_discover(conn, &inst->cli.discover_params);

	return err;
}
