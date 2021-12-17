
#include "settings.h"
#include "gui/gui_list.h"

#include "drivers/esp/protocol.h"
#include "etc/format.h"
#include "gui/bluetooth.h"

#include "gui/dialog.h"

REGISTER_TASK_ILS(bluetooth,
	lv_obj_t * info;
);



static void bluetooth_unpair_cb(dialog_result_t res, void * data)
{
	if (res == dialog_res_yes)
	{
		protocol_send(PROTO_BT_UNPAIR, NULL, 0);
	}
}

static bool bluetooth_unpair(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		dialog_show("Unbind?",
				"Do you want to unpair from all paired devices?\n",
				dialog_yes_no, bluetooth_unpair_cb);

		//supress default handler
		return false;
	}

	return true;
}

static lv_obj_t * bluetooth_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Bluetooth", &gui_settings, NULL);

	local-> info = gui_list_info_add_entry(list, "", "");

	gui_list_auto_entry(list, "Enable", &config.bluetooth.enabled, NULL);
	gui_list_auto_entry(list, "A2DP Audio", &config.bluetooth.a2dp, NULL);
	gui_list_auto_entry(list, "SPP Telemetry", &config.bluetooth.spp, NULL);
	gui_list_auto_entry(list, "BLE Telemetry", &config.bluetooth.ble, NULL);
    gui_list_auto_entry(list, "Telemetry protocol", &config.bluetooth.protocol, NULL);
    gui_list_auto_entry(list, "Forward GNSS", &config.bluetooth.forward_gnss, NULL);

	gui_list_auto_entry(list, "Device name", &config.device_name, NULL);
	gui_list_auto_entry(list, "PIN code", &config.bluetooth.pin, NULL);

	gui_list_auto_entry(list, "Unpair all", CUSTOM_CB, bluetooth_unpair);



	bluetooth_discoverable(true);
	return list;
}

static void bluetooth_loop()
{
	if (fc.esp.state & ESP_STATE_BT_ON)
	{
 		gui_list_info_set_name(local->info, "Now visible as");
 		gui_list_info_set_value(local->info, config_get_text(&config.device_name));
	}
	else
	{
 		gui_list_info_set_name(local->info, "Turn on Bluetoth");
 		gui_list_info_set_value(local->info, "to enable pairing");
	}
}

static void bluetooth_stop()
{
	bluetooth_discoverable(false);
}

