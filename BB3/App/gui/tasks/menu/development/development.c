#include <gui/tasks/menu/development/fake.h>
#include <gui/tasks/menu/development/sensors.h>
#include <gui/tasks/menu/settings.h>
#include "gui/gui_list.h"
#include "fc/fc.h"
#include "drivers/esp/esp.h"
#include "drivers/esp/protocol.h"
#include "drivers/psram.h"
#include "etc/format.h"

REGISTER_TASK_IL(development,
	lv_obj_t * esp_ext_prog;
    lv_obj_t * esp_boot0;
    lv_obj_t * usb_otg_pin;
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

//    sound_start("/data/test.wav");

//    local->slot = esp_http_get("https://strato.skybean.eu/update/devel/n48e017.hgt", DOWNLOAD_SLOT_TYPE_FILE, dev_get_file_cb);
//    local->slot = esp_http_get("http://192.168.10.32/n48e017.hgt", DOWNLOAD_SLOT_TYPE_FILE, dev_get_file_cb);

    esp_device_reset();
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

		if (obj == local->esp_boot0)
        {
            bool val = gui_list_switch_get_value(local->esp_boot0);
            esp_boot0_ctrl(val);
        }

		if (obj == local->usb_otg_pin)
        {
            bool val = gui_list_switch_get_value(local->usb_otg_pin);
            GpioWrite(BQ_OTG, val);
        }
	}

	if (event == LV_EVENT_CLICKED)
	{
        if (obj == local->trigger)
            development_trigger();
	}

	return true;
}

gui_list_slider_options_t opt =
{
	.disp_multi = 1,
	.step = 1,
	.format = format_int
};

static lv_obj_t * development_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Development", &gui_settings, development_cb);

	local->slot = 0xFF;

	bool ext_active = fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual;

    local->trigger = gui_list_text_add_entry(list, "Trigger");

    gui_list_auto_entry(list, "Sensors", NEXT_TASK, &gui_sensors);
    //gui_list_auto_entry(list, "Fake", NEXT_TASK, &gui_fake);

    gui_list_auto_entry(list, "ESP Disable", &config.debug.esp_off, NULL);
    gui_list_auto_entry(list, "ESP Watchdog", &config.debug.esp_wdt, NULL);
    local->esp_ext_prog = gui_list_switch_add_entry(list, "ESP ext prog", ext_active);
    local->esp_boot0 = gui_list_switch_add_entry(list, "ESP boot0", false);
    local->usb_otg_pin = gui_list_switch_add_entry(list, "USB OTG pin", GpioRead(BQ_OTG));
    gui_list_auto_entry(list, "Debug to serial", &config.debug.use_serial, NULL);
    gui_list_auto_entry(list, "Debug to file", &config.debug.use_file, NULL);
    gui_list_auto_entry(list, "Vario random test", &config.debug.vario_random, NULL);

	return list;
}

