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
#include "fc/fc.h"
#include "fc/telemetry/telemetry.h"
#include "etc/notifications.h"

__align static proto_mac_t dev_mac;
static bool dev_ble;

static bool dialog_on = false;

void bluetooth_pair_dialog_cb(uint8_t res, void * param)
{
	__align proto_bt_pair_res_t data;

	safe_memcpy(data.dev, dev_mac, sizeof(dev_mac));
	data.pair = false;
	data.ble = dev_ble;
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

	if (strlen(packet->dev_name) != 0)
	{
		char key[18];
		format_mac(key, packet->dev);
		db_insert(PATH_BT_NAMES, key, packet->dev_name);
	}

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

		if (fc.esp.state & (ESP_STATE_BT_BLE | ESP_STATE_BT_SPP))
			telemetry_start();

		snprintf(msg, sizeof(msg), "%s connected (%s)", get_dev_name(packet->dev, name), tag[index]);

        if (dialog_on)
        {
            dialog_close();
            dialog_on = false;
        }
        notification_send(notify_bt_connected);
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

		if ((fc.esp.state & (ESP_STATE_BT_BLE | ESP_STATE_BT_SPP)) == 0)
			telemetry_stop();

		snprintf(msg, sizeof(msg), "%s disconnected (%s)",  get_dev_name(packet->dev, name), tag[index]);

		notification_send(notify_bt_disconnected);
	}

	if (packet->mode & PROTO_BT_MODE_PAIRED)
	{
		snprintf(msg, sizeof(msg), "%s paired", get_dev_name(packet->dev, name));
	}

	statusbar_msg_add(STATUSBAR_MSG_INFO, msg);
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

    safe_memcpy(dev_mac, packet->dev, sizeof(dev_mac));
    dev_ble = packet->ble;
    dialog_on = true;

    if (packet->only_show)
    {
        snprintf(msg, sizeof(msg), "Type this passcode\n\n%lu\n\non the other device.", packet->value);
        dialog_show("Bluetooth pairing request", msg, dialog_confirm, NULL);
    }
    else
    {
        snprintf(msg, sizeof(msg), "Confirm that passcode\n\n%lu\n\nis the same on other the device.", packet->value);
        dialog_show("Bluetooth pairing request", msg, dialog_yes_no, bluetooth_pair_dialog_cb);
    }
}

void bluetooth_discoverable(bool en)
{
	INFO("bluetooth_discoverable");
	proto_bt_discoverable_t data;
	data.enabled = en;
	protocol_send(PROTO_BT_DISCOVERABLE, (void *)&data, sizeof(data));
}

