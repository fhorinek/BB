/*
 * bluetooth.c
 *
 *  Created on: 24. 9. 2021
 *      Author: horinek
 */


#include "bluetooth.h"
#include "dialog.h"
#include "drivers/esp/protocol.h"

#include "etc/format.h"
#include "gui/statusbar.h"
#include "gui/bluetooth.h"

static proto_mac_t dev_mac;
static bool dialog_on = false;

void bluetooth_pair_dialog_cb(uint8_t res, void * param)
{
	proto_bt_pair_res_t data;

	memcpy(data.dev, dev_mac, sizeof(dev_mac));
	data.pair = false;
	dialog_on = false;

	if (res == dialog_res_yes)
	{
		data.pair = true;
	}

	protocol_send(PROTO_BT_PAIR_RES, (void *)&data, sizeof(data));
}

static char * get_dev_name(uint8_t * mac, char * buff)
{
	char key[18];

	format_mac(key, mac);
	if (!db_query(PATH_BT_NAMES, key, buff, PROTO_BT_DEV_NAME_LEN))
		strcpy(buff, key);

	return buff;
}

void bluetooth_notify(proto_bt_notify_t * packet)
{
	char msg[64];
	char name[PROTO_BT_DEV_NAME_LEN];

	char * tag[] = {"A2DP", "SPP", "BLE"};
	uint8_t index;

	if (packet->mode & PROTO_BT_MODE_CONNECTED)
	{
		if (packet->mode & PROTO_BT_MODE_A2DP)
		{
			fc.esp.state |= ESP_STATE_BT_A2DP;
			index = 0;
		}

		if (packet->mode & PROTO_BT_MODE_SPP)
		{
			fc.esp.state |= ESP_STATE_BT_SPP;
			index = 1;
		}

		if (packet->mode & PROTO_BT_MODE_BLE)
		{
			fc.esp.state |= ESP_STATE_BT_BLE;
			index = 2;
		}

		snprintf(msg, sizeof(msg), "%s connected (%s)", get_dev_name(packet->dev, name), tag[index]);
	}

	if (packet->mode & PROTO_BT_MODE_DISCONNECTED)
	{
		if (packet->mode & PROTO_BT_MODE_A2DP)
		{
			fc.esp.state &= ~ESP_STATE_BT_A2DP;
			index = 0;
		}

		if (packet->mode & PROTO_BT_MODE_SPP)
		{
			fc.esp.state &= ~ESP_STATE_BT_SPP;
			index = 1;
		}

		if (packet->mode & PROTO_BT_MODE_BLE)
		{
			fc.esp.state &= ~ESP_STATE_BT_BLE;
			index = 2;
		}

		snprintf(msg, sizeof(msg), "%s disconnected (%s)",  get_dev_name(packet->dev, name), tag[index]);
	}

	if (packet->mode & PROTO_BT_MODE_PAIRED)
	{
		snprintf(msg, sizeof(msg), "%s paired", packet->dev_name);

		char key[18];
		format_mac(key, packet->dev);
		db_insert(PATH_BT_NAMES, key, packet->dev_name);
	}
	statusbar_add_msg(STATUSBAR_MSG_INFO, msg);
}

void bluetooth_pari_req(proto_bt_pair_req_t * packet)
{
	char msg[256];

	if (packet->cancel)
	{
		if (dialog_on)
		{
			dialog_close();
			dialog_on = false;
			dialog_show("Bluetooth pairing request", "pairing failed", dialog_confirm, NULL);
		}

		return;
	}

	memcpy(dev_mac, packet->dev, sizeof(dev_mac));
	dialog_on = true;

	snprintf(msg, sizeof(msg), "Confirm that passcode\n\n%lu\n\nis the same on other the device.", packet->value);
	dialog_show("Bluetooth pairing request", msg, dialog_yes_no, bluetooth_pair_dialog_cb);
}

void bluetooth_discoverable(bool en)
{
	proto_bt_discoverable_t data;
	data.enabled = en;
	protocol_send(PROTO_BT_DISCOVERABLE, (void *)&data, sizeof(data));
}

