#include "fanet.h"

#include "settings.h"

#include "../gui_list.h"

#include "../../config/config.h"
#include "../../fc/fc.h"
#include "../../etc/format.h"

typedef struct
{
	uint8_t cnt;
	uint8_t magic;

	lv_obj_t * fanet_sw;
} local_vars_t;

static void fanet_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, GUI_SW_LEFT_RIGHT);

	if (event == LV_EVENT_VALUE_CHANGED)
	{
		switch(index)
		{
			case 0:
			{
				bool val = gui_list_switch_get_value(local.fanet_sw);
				config_set_bool(&config.devices.fanet.enabled, val);
			}
			break;
		}

	}
}



static lv_obj_t * fanet_init(lv_obj_t * par)
{
	local.cnt = 0;
	local.magic = 0;

	lv_obj_t * list = gui_list_create(par, "FANET Settings", fanet_cb);

	local.fanet_sw = gui_list_switch_add_entry(list, "Enable FANET", config_get_bool(&config.devices.fanet.enabled));

	return list;
}

static void fanet_loop()
{
	if (local.magic != fc.fanet.neighbors_magic)
	{
		local.magic = fc.fanet.neighbors_magic;
		while (local.cnt < fc.fanet.neighbors_size)
		{
			gui_list_info_add_entry(gui_list, "", "");
			local.cnt++;
		}

		while (local.cnt  > fc.fanet.neighbors_size)
		{
	//		gui_list_del_entry()
			local.cnt--;
		}

		for (uint8_t i = 0; i < local.cnt; i++)
		{
			neighbor_t * nb = &fc.fanet.neighbor[i];
			char name[32];
			char dist[16];

			sprintf(name, "%02X:%04X", nb->addr.manufacturer_id, nb->addr.user_id);
			if (nb->name[0] != 0)
				sprintf(name + strlen(name), ": %s", nb->name);

			format_distance(dist, nb->dist);
			sprintf(dist + strlen(dist), " @%lus", (HAL_GetTick() / 1000) - nb->timestamp);

			lv_obj_t * entry = gui_list_get_entry(i + 1);
			gui_list_info_set_value(entry, dist);
			gui_list_info_set_name(entry, name);
		}
	}
}

static bool fanet_stop()
{
	return true;
}

gui_task_t gui_fanet =
{
	fanet_init,
	fanet_loop,
	fanet_stop
};
