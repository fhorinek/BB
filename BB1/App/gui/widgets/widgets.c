/*
 * widgets.cc
 *
 *  Created on: 10. 8. 2020
 *      Author: horinek
 */

#include "widgets.h"
#include "fatfs.h"

bool widgets_load_from_file(lv_obj_t * base, page_layout_t * page, char * name)
{
	FIL f;
	char path[64];
	uint8_t ret;

	snprintf(path, sizeof(path), "%s/%s.pag", PATH_PAGES_DIR, name);
	if ((ret = f_open(&f, path, FA_READ)) != FR_OK)
	{
		ERR("Unable to open %s Error:%u", path, ret);
		return false;
	}

	char buff[255];

	page->number_of_widgets = atoi(find_in_file(&f, "widgets", "0", buff, sizeof(buff)));

	if (page->number_of_widgets > 0)
	{
		page->widgets = (widget_slot_t **) malloc(sizeof(widget_slot_t *) * page->number_of_widgets);
		ASSERT(page->widgets != NULL);

		for (uint8_t i = 0; i < page->number_of_widgets; i++)
		{
			widget_slot_t * w = malloc(sizeof(widget_slot_t));
			char key[16];

			//Get widget type
			snprintf(key, sizeof(key), "w%u_type", i);
			char * type = find_in_file(&f, key, "0", buff, sizeof(buff));

			w->widget = NULL;
			for (uint8_t i = 0; i < number_of_widgets(); i++)
			{
				if (strcmp(type, widgets[i]->short_name) == 0)
				{
					w->widget = widgets[i];
					break;
				}
			}

			if (w->widget == NULL)
			{
				w->widget = widgets[0];
				WARN("widget[%u] type '%s' unknown", i, type);
			}

			//get widget parameters
			snprintf(key, sizeof(key), "w%u_x", i);
			int16_t x = atoi(find_in_file(&f, key, "0", buff, sizeof(buff)));

			snprintf(key, sizeof(key), "w%u_y", i);
			int16_t y = atoi(find_in_file(&f, key, "0", buff, sizeof(buff)));

			snprintf(key, sizeof(key), "w%u_w", i);
			int16_t width = atoi(find_in_file(&f, key, "0", buff, sizeof(buff)));

			snprintf(key, sizeof(key), "w%u_h", i);
			int16_t height = atoi(find_in_file(&f, key, "0", buff, sizeof(buff)));

			//widget specific init
			w->widget->init(base, w, x, y, width, height);
		}
	}

	f_close(&f);

	return true;
}

void widgets_unload(page_layout_t * page)
{
	for (uint8_t i = 0; i < page->number_of_widgets; i++)
	{
		//widget specific stop
		page->widgets[i]->widget->stop(page->widgets[i]);

		//free widget memory
		free(page->widgets[i]);
	}

	//free widget slot pointer memory
	if (page->number_of_widgets > 0)
		free(page->widgets);
}

