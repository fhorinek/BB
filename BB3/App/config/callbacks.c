#include "config.h"

#include "drivers/gnss/fanet.h"
#include "drivers/esp/esp.h"
#include "drivers/esp/protocol.h"
#include "drivers/rev.h"
#include "drivers/power/led.h"

#include "gui/gui_list.h"
#include "gui/dbg_overlay.h"
#include "gui/statusbar.h"

#include "etc/bootloader.h"

//trigger when new version is available, from gui pages
void config_new_version_cb()
{
    if (file_exists(PATH_NEW_FW))
    {
        f_unlink(PATH_NEW_FW);

        release_note_show();

        if (bootloader_update(PATH_BL_FW_AUTO) == bl_update_ok)
        {
            statusbar_msg_add(STATUSBAR_MSG_INFO, "Bootloader successfully updated!");
        }

        f_unlink(PATH_BL_FW_AUTO);
    }
}

static void dev_name_cb(cfg_entry_t * entry)
{
    if (strlen(config_get_text(entry)) == 0)
    {
        char dev_name[DEV_NAME_LEN];
        sprintf(dev_name, "Strato_%lX", rev_get_short_id());
        config_set_text(entry, dev_name);
    }
}

static void wifi_pass_cb(cfg_entry_t * entry)
{
    if (strlen(config_get_text(entry)) < 8)
    {
        config_set_text(entry, "");
    }
}

static void fanet_enable_cb(cfg_entry_t * entry)
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

static void flarm_enable_cb(cfg_entry_t * entry)
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

static void flarm_config_cb(cfg_entry_t * entry)
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

static void dbg_esp_off_cb(cfg_entry_t * entry)
{
	if (config_get_bool(entry))
		esp_deinit();
	else
		esp_init();
}

static void dbg_esp_tasks_cb(cfg_entry_t * entry)
{
    if (config_get_select(entry) != DBG_TASK_NONE)
        dbg_overlay_tasks_create();
    else
        dbg_overlay_tasks_remove();
}

static void wifi_mode_cb(cfg_entry_t * entry)
{
	esp_set_wifi_mode();
}

static void bt_volume_cb(cfg_entry_t * entry)
{
	uint8_t vol = config_get_int(entry);

	uint8_t ch;
	if (entry == &profile.audio.master_volume)
		ch = PROTO_VOLUME_MASTER;
	if (entry == &profile.audio.a2dp_volume)
		ch = PROTO_VOLUME_A2DP;
	if (entry == &profile.audio.vario_volume)
		ch = PROTO_VOLUME_VARIO;
	if (entry == &profile.audio.sound_volume)
		ch = PROTO_VOLUME_SOUND;

	esp_set_volume(ch, vol);
}

static void bt_mode_cb(cfg_entry_t * entry)
{
	if (entry != &profile.bluetooth.enabled)
	{
		if (config_get_bool(&profile.bluetooth.a2dp) != false
				|| config_get_bool(&profile.bluetooth.spp) != false
				|| config_get_bool(&profile.bluetooth.ble) != false)
		{
			config_set_bool(&profile.bluetooth.enabled, true);
		}
	}

	if (config_get_bool(&profile.bluetooth.a2dp) == false
			&& config_get_bool(&profile.bluetooth.spp) == false
			&& config_get_bool(&profile.bluetooth.ble) == false)
	{
		config_set_bool(&profile.bluetooth.enabled, false);
	}

	esp_reboot();
}

static void disp_bck_cb(cfg_entry_t * entry)
{
	uint8_t val = config_get_int(entry);
	led_set_backlight(val);
}

static void dbg_usb_cb(cfg_entry_t * entry)
{
	bool val = config_get_bool(entry);
	INFO("USB debug is now %sabled", val ? "en" : "dis");
}

static void updates_cb(cfg_entry_t * entry)
{
    if (!config_get_bool(entry))
        statusbar_set_icon(BAR_ICON_FW, I_HIDE);
}

cfg_callback_pair_t config_callbacks[] =
{
    {&config.device_name, dev_name_cb},
    {&config.system.check_for_updates, updates_cb},
	{&profile.audio.master_volume, bt_volume_cb},
	{&profile.audio.a2dp_volume, bt_volume_cb},
	{&profile.audio.vario_volume, bt_volume_cb},
	{&profile.audio.sound_volume, bt_volume_cb},
	{&profile.bluetooth.enabled, bt_mode_cb},
	{&profile.bluetooth.a2dp, bt_mode_cb},
	{&profile.bluetooth.spp, bt_mode_cb},
	{&profile.bluetooth.ble, bt_mode_cb},
    {&config.wifi.ap_pass, wifi_pass_cb},
    {&profile.wifi.enabled, wifi_mode_cb},
    {&profile.wifi.ap, wifi_mode_cb},
	{&config.display.backlight, disp_bck_cb},
    {&config.debug.esp_off, dbg_esp_off_cb},
    {&config.debug.tasks, dbg_esp_tasks_cb},
	{&config.debug.use_usb, dbg_usb_cb},
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
    gui_config_config_cb(entry);

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
