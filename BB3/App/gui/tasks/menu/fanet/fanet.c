#include "fanet.h"

#include <gui/tasks/menu/settings.h>
#include "settings.h"

#include "gui/gui_list.h"
#include "fc/fc.h"
#include "etc/format.h"

REGISTER_TASK_IL(fanet,
	uint8_t cnt;
	uint8_t magic;
);

static bool fanet_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{

	return true;
}



static lv_obj_t * fanet_init(lv_obj_t * par)
{
	local->cnt = 0;
	local->magic = 0;

	lv_obj_t * list = gui_list_create(par, "FANET neighbors", &gui_settings, fanet_cb);

	gui_list_auto_entry(list, "Settings", NEXT_TASK, &gui_fanet_settings);

	return list;
}

static void fanet_loop()
{
	if (local->magic != fc.fanet.neighbors_magic)
	{
		local->magic = fc.fanet.neighbors_magic;

		//add new entry
		while (local->cnt < fc.fanet.neighbors_size)
		{
			gui_list_info_add_entry(gui.list.object, "", "");
			local->cnt++;
		}

		//remove entry
		for (uint8_t i = fc.fanet.neighbors_size; i < local->cnt; i++)
		{
			lv_obj_t * entry = gui_list_get_entry(i + 1);
			lv_obj_del(entry);

			local->cnt--;
		}

		//update entry
		for (uint8_t i = 0; i < local->cnt; i++)
		{
			neighbor_t * nb = &fc.fanet.neighbor[i];
			char name[32];
			char tmp[16];
			char text[32];

			sprintf(name, "%02X:%04X", nb->addr.manufacturer_id, nb->addr.user_id);
			if (nb->name[0] != 0)
				sprintf(name, "%02X:%04X: %s", nb->addr.manufacturer_id, nb->addr.user_id, nb->name);
			else
				sprintf(name, "%02X:%04X", nb->addr.manufacturer_id, nb->addr.user_id);

			strcpy(text, "");
 			if (nb->flags & NB_HAVE_POS)
 			{
				format_distance_with_units(tmp, (float)nb->dist);
				strcat(text, tmp);
 			}

			if (nb->max_dist != 0)
			{
				if (nb->flags & NB_HAVE_POS)
				{
					char slash[] = " / ";
					strcat(text, slash);
				}
				format_distance_with_units(tmp, (float)nb->max_dist);
				strcat(text, tmp);
			}

			sprintf(tmp, " @%lus", (HAL_GetTick() / 1000) - nb->timestamp);
			strcat(text, tmp);

			lv_obj_t * entry = gui_list_get_entry(i + 1);
			gui_list_info_set_value(entry, text);
			gui_list_info_set_name(entry, name);
		}
	}
}

