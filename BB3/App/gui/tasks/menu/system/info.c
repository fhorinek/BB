/*
 * info.c
 *
 *  Created on: Apr 30, 2021
 *      Author: horinek
 */


#include "gui/gui_list.h"

#include "system.h"

#include "fc/fc.h"

#include "drivers/rev.h"
#include "drivers/esp/download/slot.h"
#include "drivers/esp/protocol.h"
#include "drivers/power/pwr_mng.h"

#include "gui/dialog.h"
#include "gui/tasks/filemanager.h"
#include "gui/statusbar.h"
#include "gui/statusbar.h"

#include "etc/format.h"
#include "etc/bootloader.h"


REGISTER_TASK_IL(info,
    uint8_t click_cnt;
    lv_obj_t * fanet;
    lv_obj_t * sta_mac;
    lv_obj_t * ap_mac;
    lv_obj_t * bt_mac;

);

static bool info_serial_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
    	local->click_cnt++;
    	if (local->click_cnt > 5)
    	{
    		local->click_cnt = 0;

    		if (DEVEL_ACTIVE)
    		{
    			red_unlink(DEV_MODE_FILE);
    			statusbar_msg_add(STATUSBAR_MSG_INFO, "Developer mode disabled");
    		}
    		else
    		{
    			touch(DEV_MODE_FILE);
    			statusbar_msg_add(STATUSBAR_MSG_INFO, "Developer mode enabled");
    		}

    	}
    }
    return true;
}

void info_loop()
{
    char tmp[32];

    format_mac(tmp, fc.esp.mac_sta);
    gui_list_info_set_value(local->sta_mac, tmp);

    format_mac(tmp, fc.esp.mac_ap);
    gui_list_info_set_value(local->ap_mac, tmp);

    format_mac(tmp, fc.esp.mac_bt);
    gui_list_info_set_value(local->bt_mac, tmp);

    if (fc.fanet.flarm_expires == 0)
    {
        strcpy(tmp, "N/A");
        gui_list_info_set_value(local->fanet, tmp);
    }
    else
    {
        snprintf(tmp, sizeof(tmp), "%02X%04X", fc.fanet.addr.manufacturer_id, fc.fanet.addr.user_id);
        gui_list_info_set_value(local->fanet, tmp);
    }
}

lv_obj_t * info_init(lv_obj_t * par)
{
	local->click_cnt = 0;

    lv_obj_t * list = gui_list_create(par, "Device info", &gui_system, NULL);

    char value[32];
    lv_obj_t * obj;

    snprintf(value, sizeof(value), "%08lX", rev_get_short_id());
    obj = gui_list_info_add_entry(list, "Serial number", value);
    gui_config_entry_add(obj, CUSTOM_CB, info_serial_cb);

    snprintf(value, sizeof(value), "%02X", rev_get_hw());
    gui_list_info_add_entry(list, "Hardware revision", value);

    local->fanet = gui_list_info_add_entry(list, "FANET ID", "");
    local->sta_mac = gui_list_info_add_entry(list, "Wi-Fi MAC", "");
    local->ap_mac = gui_list_info_add_entry(list, "Wi-Fi AP MAC", "");
    local->bt_mac = gui_list_info_add_entry(list, "Bluetooth MAC", "");


    return list;
}
