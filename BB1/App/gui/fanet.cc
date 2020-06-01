#include "fanet.h"

#include "settings.h"

#include "gui_list.h"

#include "../config/config.h"
#include "../fc/fc.h"
#include "../etc/format.h"

void fanet_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, GUI_SW_LEFT_RIGHT);

	if (event == LV_EVENT_VALUE_CHANGED)
	{
		switch(index)
		{
			case 0:
			{
				bool val = gui_list_switch_get_value(index);
				config_set_bool(&config.devices.fanet.enabled, val);
			}
			break;
		}

	}
}


lv_obj_t * fanet_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "FANET Settings", fanet_cb);

	gui_list_switch_add_entry(list, "Enable FANET", config_get_bool(&config.devices.fanet.enabled));

	return list;
}

void fanet_loop()
{
	static uint8_t last_cnt = 0;
	static uint8_t last_magic = 0;

	if (last_magic != fc.fanet.neighbors_magic)
	{
		last_magic = fc.fanet.neighbors_magic;
		while (last_cnt < fc.fanet.neighbors_size)
		{
			gui_list_info_add_entry(gui_list, "", "");
			last_cnt++;
		}

		while (last_cnt > fc.fanet.neighbors_size)
		{
	//		gui_list_del_entry()
			last_cnt--;
		}

		for (uint8_t i = 0; i < last_cnt; i++)
		{
			neighbor_t * nb = &fc.fanet.neighbor[i];
			char name[32];
			char dist[8];

			sprintf(name, "%02X:%04X", nb->addr.manufacturer_id, nb->addr.user_id);
			if (nb->name[0] != 0)
				sprintf(name + strlen(name), ": %s", nb->name);

			format_distance(dist, nb->dist);

			gui_list_info_set_value(i + 1, dist);
			gui_list_info_set_name(i + 1, name);
		}
	}
}

bool fanet_stop()
{
	return true;
}

gui_task_t gui_fanet =
{
	fanet_init,
	fanet_loop,
	fanet_stop
};
