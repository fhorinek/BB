#include "config.h"

#include "drivers/gnss/fanet.h"
#include "drivers/esp/esp.h"
#include "drivers/esp/protocol.h"
#include "drivers/rev.h"
#include "drivers/power/led.h"

#include "gui/gui_list.h"

 void dev_name_cb(cfg_entry_t * entry)
{
    if (strlen(config_get_text(entry)) == 0)
    {
        char dev_name[DEV_NAME_LEN];
        sprintf(dev_name, "Strato_%lX", rev_get_short_id());
        config_set_text(entry, dev_name);
    }
}

void wifi_pass_cb(cfg_entry_t * entry)
{
    if (strlen(config_get_text(entry)) < 8)
    {
        config_set_text(entry, "");
    }
}

void fanet_enable_cb(cfg_entry_t * entry)
{
	if (config_get_bool(entry))
	{
		fanet_enable();
	}
	else
	{
		fanet_disable();
	}
}

void flarm_enable_cb(cfg_entry_t * entry)
{
	if (config_get_select(&profile.fanet.air_type) != FANET_AIRCRAFT_TYPE_PARAGLIDER
		&& config_get_select(&profile.fanet.air_type) != FANET_AIRCRAFT_TYPE_HANGGLIDER)
	{
		config_disable_callbacks();
		config_set_select(&profile.fanet.air_type, FANET_AIRCRAFT_TYPE_PARAGLIDER);
		config_enable_callbacks();
	}

	fanet_configure_flarm(false);
}

void flarm_config_cb(cfg_entry_t * entry)
{
	if (config_get_select(&profile.fanet.air_type) != FANET_AIRCRAFT_TYPE_PARAGLIDER
		&& config_get_select(&profile.fanet.air_type) != FANET_AIRCRAFT_TYPE_HANGGLIDER)
	{
		config_disable_callbacks();
		config_set_bool(&profile.fanet.flarm, false);
		config_enable_callbacks();
	}

	fanet_configure_type(false);
}

void dbg_esp_off_cb(cfg_entry_t * entry)
{
	if (config_get_bool(entry))
		esp_deinit();
	else
		esp_init();
}

void wifi_mode_cb(cfg_entry_t * entry)
{
	esp_set_wifi_mode();
}

void bt_volume_cb(cfg_entry_t * entry)
{
	uint8_t vol = config_get_int(entry);
	esp_set_volume(vol);
}

void disp_bck_cb(cfg_entry_t * entry)
{
	uint8_t val = config_get_int(entry);
	led_set_backlight(val);
}

cfg_callback_pair_t config_callbacks[] =
{
    {&config.device_name, dev_name_cb},
	{&config.bluetooth.volume, bt_volume_cb},
    {&config.wifi.ap_pass, wifi_pass_cb},
    {&config.wifi.enabled, wifi_mode_cb},
    {&config.wifi.ap, wifi_mode_cb},
	{&config.display.backlight, disp_bck_cb},
	{&config.debug.esp_off, dbg_esp_off_cb},
    {&profile.fanet.enabled, fanet_enable_cb},
    {&profile.fanet.flarm, flarm_enable_cb},
    {&profile.fanet.air_type, flarm_config_cb},
    {&profile.fanet.ground_type, flarm_config_cb},
    {&pilot.online_track, flarm_config_cb},
};

static bool config_callbacks_enabled = true;

void config_process_cb(cfg_entry_t * entry)
{
    if (config_callbacks_enabled)
    {
		for (uint16_t i = 0; i < sizeof(config_callbacks) / sizeof(cfg_callback_pair_t); i++)
		{
			if (entry == config_callbacks[i].entry)
			{
				config_callbacks[i].cb(entry);
				break;
			}
		}
    }

    //update gui
    gui_lock_acquire();
    gui_config_config_cb(entry);
    gui_lock_release();

    config_changed = true;
}

void config_trigger_callbacks()
{
    for (uint16_t i = 0; i < sizeof(config_callbacks) / sizeof(cfg_callback_pair_t); i++)
    {
        config_callbacks[i].cb(config_callbacks[i].entry);
    }
}

void config_disable_callbacks()
{
    config_callbacks_enabled = false;
}

void config_enable_callbacks()
{
    config_callbacks_enabled = true;
}
