#include "settings.h"
#include "gui/tasks/menu/fanet/fanet.h"

#include "gui/gui_list.h"
#include "fc/fc.h"
#include "etc/format.h"
#include "drivers/rtc.h"

REGISTER_TASK_IL(fanet_settings,
    lv_obj_t * id;
    lv_obj_t * ver;
    lv_obj_t * exp;
);

static lv_obj_t * fanet_settings_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "FANET settings", &gui_fanet, NULL);

    gui_list_auto_entry(list, "Enable FANET", &profile.fanet.enabled, NULL);
    gui_list_auto_entry(list, "Enable FLARM", &profile.fanet.flarm, NULL);
    gui_list_auto_entry(list, "Airborne type", &profile.fanet.air_type, NULL);
    gui_list_auto_entry(list, "Ground type", &profile.fanet.ground_type, NULL);
    gui_list_auto_entry(list, "Show labels", &profile.fanet.show_labels, NULL);

    local->id = gui_list_info_add_entry(list, "FANET ID", "");
    local->ver = gui_list_info_add_entry(list, "FANET Firmware", "");
    local->exp = gui_list_info_add_entry(list, "FLARM expires", "");

    return list;
}


static void fanet_settings_loop()
{
    char value[64];

    if (fc.fanet.flarm_expires == 0)
    {
        strcpy(value, "N/A");
        gui_list_info_set_value(local->id, value);
        gui_list_info_set_value(local->ver, value);
        gui_list_info_set_value(local->exp, value);
    }
    else
    {
        snprintf(value, sizeof(value), "%02X%04X", fc.fanet.addr.manufacturer_id, fc.fanet.addr.user_id);
        gui_list_info_set_value(local->id, value);

        strcpy(value, fc.fanet.version);
        gui_list_info_set_value(local->ver, value);

        char tmp[16];
        format_date_epoch(tmp, fc.fanet.flarm_expires);
        if (rtc_is_valid())
        {
            int32_t delta = ((int64_t)fc.fanet.flarm_expires - (int64_t)fc_get_utc_time()) / (24 * 3600);
            if (delta > 0)
                snprintf(value, sizeof(value), "%s in %ld days", tmp, delta);
            else
                snprintf(value, sizeof(value), "%s EXPIRED!", tmp);
        }
        else
        {
            strncpy(value, tmp, sizeof(value));
        }
        gui_list_info_set_value(local->exp, value);
    }
}
