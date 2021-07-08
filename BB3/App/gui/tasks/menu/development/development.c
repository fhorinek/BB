#include <gui/tasks/menu/development/fake.h>
#include <gui/tasks/menu/development/sensors.h>
#include <gui/tasks/menu/fanet.h>
#include <gui/tasks/menu/settings.h>
#include "gui/gui_list.h"
#include "fc/fc.h"
#include "drivers/esp/esp.h"
#include "drivers/esp/protocol.h"
#include "drivers/psram.h"


REGISTER_TASK_IL(development,
	lv_obj_t * esp_ext_prog;
    lv_obj_t * esp_boot0;
    lv_obj_t * usb_otg_pin;
    lv_obj_t * use_serial;
    lv_obj_t * use_file;
);

void development_trigger()
{
    INFO("Development fake trigger");
    INFO("-----------------------------------------------------");
//    uint8_t data[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xDF};
//
//    esp_spi_send(data, sizeof(data));
//    esp_spi_prepare();

//    sound_start("/data/test.wav");

//    download_slot_init();
//    esp_download_file("https://vps.skybean.eu/strato/", dev_ds_cb);
}

static void development_loop()
{
    bool ext_active = fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual;
    bool val = gui_list_switch_get_value(local->esp_ext_prog);

    if (val != ext_active)
        gui_list_switch_set_value(local->esp_ext_prog, ext_active);
}

static bool development_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
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

		if (obj == local->use_serial)
		{
			config_set_bool(&config.debug.use_serial, gui_list_switch_get_value(local->use_serial));
		}

		if (obj == local->use_file)
		{
			config_set_bool(&config.debug.use_file, gui_list_switch_get_value(local->use_file));
		}
	}

	if (event == LV_EVENT_CLICKED)
	{
        if (index == 0)
            development_trigger();

        if (index == 4)
            gui_switch_task(&gui_sensors, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 5)
            gui_switch_task(&gui_fake, LV_SCR_LOAD_ANIM_MOVE_LEFT);
	}

	return true;
}



static lv_obj_t * development_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Develpment", &gui_settings, development_cb);

	bool ext_active = fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual;

    gui_list_text_add_entry(list, "Trigger");
    local->esp_ext_prog = gui_list_switch_add_entry(list, "ESP ext prog", ext_active);
    local->esp_boot0 = gui_list_switch_add_entry(list, "ESP boot0", false);
    local->usb_otg_pin = gui_list_switch_add_entry(list, "USB OTG pin", GpioRead(BQ_OTG));
    gui_list_text_add_entry(list, "Sensors");
    gui_list_text_add_entry(list, "Fake");

    local->use_serial = gui_list_switch_add_entry(list, "Debug to serial", config_get_bool(&config.debug.use_serial));
    local->use_file = gui_list_switch_add_entry(list, "Debug to file", config_get_bool(&config.debug.use_file));

	return list;
}

