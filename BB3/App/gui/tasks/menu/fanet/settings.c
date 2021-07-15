#include "settings.h"
#include "gui/tasks/menu/fanet/fanet.h"

#include "gui/gui_list.h"
#include "fc/fc.h"
#include "etc/format.h"

REGISTER_TASK_I(fanet_settings);

static lv_obj_t * fanet_settings_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "FANET settings", &gui_fanet, NULL);

    gui_list_auto_entry(list, "Enable FANET", &profile.fanet.enabled, NULL);
    gui_list_auto_entry(list, "Enable FLARM", &profile.fanet.flarm, NULL);
    gui_list_auto_entry(list, "Airborne type", &profile.fanet.air_type, NULL);
    gui_list_auto_entry(list, "Ground type", &profile.fanet.ground_type, NULL);

    char value[32];
    snprintf(value, sizeof(value), "%02X%04X", fc.fanet.addr.manufacturer_id, fc.fanet.addr.user_id);
    gui_list_info_add_entry(list, "FANET ID", value);

    if (strlen(fc.fanet.version) == 0)
    	strcpy(value, "N/A");
    else
    	strcpy(value, fc.fanet.version);

    gui_list_info_add_entry(list, "FANET Version", value);

    if (fc.fanet.flarm_expires == 0)
    	strcpy(value, "N/A");
    else
    {
    	char tmp[16];
    	format_date_epoch(tmp, fc.fanet.flarm_expires);
    	int64_t delta = (fc.fanet.flarm_expires - fc_get_utc_time()) / (24 * 3600);
    	if (delta > 0)
    		snprintf(value, sizeof(value), "%s in %u days", tmp, delta);
    	else
    		snprintf(value, sizeof(value), "%s EXPIRED!", tmp, delta);
    }

    gui_list_info_add_entry(list, "FLARM expires", value);

    return list;
}


