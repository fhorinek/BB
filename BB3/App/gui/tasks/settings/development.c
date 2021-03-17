#include "fanet.h"

#include "settings.h"
#include "sensors.h"
#include "fake.h"

#include "gui/gui_list.h"
#include "fc/fc.h"
#include "drivers/esp/esp.h"
#include "drivers/esp/sound/sound.h"

REGISTER_TASK_IL(development,
	lv_obj_t * esp_ext_prog;
    lv_obj_t * usb_otg_pin;
);

uint16_t esp_spi_send(uint8_t * data, uint16_t len);

void development_trigger()
{
    INFO("Development fake trigger");
    INFO("-----------------------------------------------------");
//    uint8_t data[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xDF};
//
//    esp_spi_send(data, sizeof(data));
//    esp_spi_prepare();

//    sound_start("/data/test.wav");
}

static void development_loop()
{
    bool ext_active = fc.esp.mode == esp_external;
    bool val = gui_list_switch_get_value(local->esp_ext_prog);

    if (val != ext_active)
        gui_list_switch_set_value(local->esp_ext_prog, ext_active);
}

static void development_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
	{
		gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
	}

	if (event == LV_EVENT_VALUE_CHANGED)
	{
		switch(index)
		{
			case 1:
			{
				bool val = gui_list_switch_get_value(local->esp_ext_prog);
				bool ext_active = fc.esp.mode == esp_external;

				if (val != ext_active)
				{
                    if (val)
                    {
                        esp_enable_external_programmer();
                    }
                    else
                    {
                        esp_disable_external_programmer();
                    }
				}
			}
			break;

            case 2:
            {
                bool val = gui_list_switch_get_value(local->usb_otg_pin);
                GpioWrite(BQ_OTG, val);
            }
            break;
		}


	}

	if (event == LV_EVENT_CLICKED)
	{
        if (index == 0)
            development_trigger();

        if (index == 3)
            gui_switch_task(&gui_sensors, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 4)
            gui_switch_task(&gui_fake, LV_SCR_LOAD_ANIM_MOVE_LEFT);
	}
}



static lv_obj_t * development_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Develpment", development_cb);

    gui_list_text_add_entry(list, "Trigger");
    local->esp_ext_prog = gui_list_switch_add_entry(list, "ESP ext prog", fc.esp.mode == esp_external);
    local->usb_otg_pin = gui_list_switch_add_entry(list, "USB OTG pin", GpioRead(BQ_OTG));
    gui_list_text_add_entry(list, "Sensors");
    gui_list_text_add_entry(list, "Fake");

	return list;
}

