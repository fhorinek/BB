#include <gui/tasks/menu/development/fake.h>
#include <gui/tasks/menu/development/sensors.h>
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
    lv_obj_t * trigger;
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
	}

	if (event == LV_EVENT_CLICKED)
	{
        if (obj == local->trigger)
            development_trigger();
	}

	return true;
}



static lv_obj_t * development_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Develpment", &gui_settings, development_cb);

	bool ext_active = fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual;

    local->trigger = gui_list_text_add_entry(list, "Trigger");
    gui_list_auto_entry(list, "Sensors", NEXT_TASK, &gui_sensors);
    gui_list_auto_entry(list, "Fake", NEXT_TASK, &gui_fake);

    gui_list_auto_entry(list, "Disable ESP32", &config.debug.esp_off, NULL);
    local->esp_ext_prog = gui_list_switch_add_entry(list, "ESP ext prog", ext_active);
    local->esp_boot0 = gui_list_switch_add_entry(list, "ESP boot0", false);
    local->usb_otg_pin = gui_list_switch_add_entry(list, "USB OTG pin", GpioRead(BQ_OTG));
    gui_list_auto_entry(list, "Debug to serial", &config.debug.use_serial, NULL);
    gui_list_auto_entry(list, "Debug to file", &config.debug.use_file, NULL);

	return list;
}

