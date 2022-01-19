#include <gui/tasks/menu/development/fake.h>
#include <gui/tasks/menu/development/sensors.h>
#include <gui/tasks/menu/settings.h>
#include "gui/gui_list.h"
#include "fc/fc.h"
#include "drivers/esp/esp.h"
#include "drivers/esp/protocol.h"
#include "drivers/psram.h"
#include "etc/format.h"
#include "gui/dialog.h"
#include "drivers/gnss/gnss_ublox_m8.h"

REGISTER_TASK_IL(development,
	lv_obj_t * esp_ext_prog;
    lv_obj_t * esp_boot0;
    lv_obj_t * trigger;

    uint8_t slot;
);

void dev_get_file_cb(uint8_t res, download_slot_t * ds)
{
	switch(res)
	{
		case(DOWNLOAD_SLOT_COMPLETE):
			INFO("DOWNLOAD_SLOT_COMPLETE");
		break;
		case(DOWNLOAD_SLOT_PROGRESS):
			INFO("DOWNLOAD_SLOT_PROGRESS %lu/%lu", ds->pos, ds->size);
		break;
		case(DOWNLOAD_SLOT_NO_CONNECTION):
			INFO("DOWNLOAD_SLOT_NO_CONNECTION");
		break;
		case(DOWNLOAD_SLOT_NOT_FOUND):
			INFO("DOWNLOAD_SLOT_NOT_FOUND");
		break;
		case(DOWNLOAD_SLOT_NO_SLOT):
			INFO("DOWNLOAD_SLOT_NO_SLOT");
		break;
		case(DOWNLOAD_SLOT_TIMEOUT):
			INFO("DOWNLOAD_SLOT_TIMEOUT");
		break;
		case(DOWNLOAD_SLOT_CANCEL):
			INFO("DOWNLOAD_SLOT_CANCEL");
		break;
	}

}

void development_trigger()
{
    INFO("Development fake trigger");
    INFO("-----------------------------------------------------");

//    ublox_init();

//    sound_start("/data/test.wav");

//    local->slot = esp_http_get("https://strato.skybean.eu/update/devel/n48e017.hgt", DOWNLOAD_SLOT_TYPE_FILE, dev_get_file_cb);
//    local->slot = esp_http_get("http://192.168.10.32/n48e017.hgt", DOWNLOAD_SLOT_TYPE_FILE, dev_get_file_cb);

//    esp_device_reset();

    protocol_send(PROTO_GET_TASKS, NULL, 0);
}

static void development_loop()
{
    bool ext_active = fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual;
    bool val = gui_list_switch_get_value(local->esp_ext_prog);

    if (val != ext_active)
        gui_list_switch_set_value(local->esp_ext_prog, ext_active);
}

static bool development_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		if (obj == local->esp_ext_prog)
        {
            bool val = gui_list_switch_get_value(local->esp_ext_prog);
            bool ext_active = fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual;

            if (val != ext_active)
            {
                if (val)
                {
                    esp_enable_external_programmer(esp_external_manual);
                }
                else
                {
                    esp_disable_external_programmer();
                }
            }
        }

	}

	if (event == LV_EVENT_CLICKED)
	{
        if (obj == local->trigger)
            development_trigger();
	}

	return true;
}

void development_clear_debug_file_dialog_cb(uint8_t res, void * data)
{
	if (res == dialog_res_yes)
	{
		f_unlink(DEBUG_FILE);
	}
}

static bool development_clear_debug_file_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		dialog_show("Confirm", "Clear debug file", dialog_yes_no, development_clear_debug_file_dialog_cb);
	}

	return true;
}

extern bool fanet_need_update;
static bool fanet_force_update_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        INFO("FANET force update");
        fanet_need_update = true;
        fc.fanet.status = fc_dev_init;

        //reset module
        fanet_enable();
    }

    return true;
}

static lv_obj_t * development_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Reset GNSS", &gui_settings, development_cb);

	local->slot = 0xFF;

	bool ext_active = fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual;

    local->trigger = gui_list_text_add_entry(list, "Trigger");

    gui_list_auto_entry(list, "Sensors", NEXT_TASK, &gui_sensors);
    //gui_list_auto_entry(list, "Fake", NEXT_TASK, &gui_fake);

    gui_list_auto_entry(list, "ESP Disable", &config.debug.esp_off, NULL);
    gui_list_auto_entry(list, "ESP Watchdog", &config.debug.esp_wdt, NULL);
    gui_list_auto_entry(list, "Show tasks", &config.debug.tasks, NULL);
    local->esp_ext_prog = gui_list_switch_add_entry(list, "ESP ext prog", ext_active);
    gui_list_auto_entry(list, "Debug to serial", &config.debug.use_serial, NULL);
    gui_list_auto_entry(list, "Debug to file", &config.debug.use_file, NULL);
    gui_list_auto_entry(list, "Clear debug.log", CUSTOM_CB, development_clear_debug_file_cb);
    gui_list_auto_entry(list, "Debug to USB", &config.debug.use_usb, NULL);
    gui_list_auto_entry(list, "Vario random test", &config.debug.vario_random, NULL);
    gui_list_auto_entry(list, "FANET force update", CUSTOM_CB, fanet_force_update_cb);

	return list;
}

