/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "pages.h"

#include "gui_list.h"
#include "settings.h"

static void pages_event_cb(lv_obj_t * obj, lv_event_t event)
{
    switch(event) {
        case LV_EVENT_PRESSED:
            DBG("Pressed\n");
            break;

        case LV_EVENT_SHORT_CLICKED:
        	DBG("Short clicked\n");
            break;

        case LV_EVENT_CLICKED:
        	DBG("Clicked\n");
        	gui_switch_task(&gui_settings, GUI_SW_RIGHT_LEFT);
            break;

        case LV_EVENT_LONG_PRESSED:
        	DBG("Long press\n");
            break;

        case LV_EVENT_LONG_PRESSED_REPEAT:
        	DBG("Long press repeat\n");
            break;

        case LV_EVENT_RELEASED:
        	DBG("Released\n");
            break;
    }

       /*Etc.*/
}

lv_obj_t * pages_init(lv_obj_t * par)
{
	lv_obj_t * tile = lv_obj_create(par, NULL);
	lv_obj_set_size(tile, LV_HOR_RES, LV_VER_RES);

	char text[32];
	sprintf(text, "PAGES\nPlaceholder");
	lv_obj_t * label = lv_label_create(tile, NULL);
	lv_label_set_text(label, text);
	lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

	lv_group_add_obj(gui_group, label);
	lv_obj_set_event_cb(label, pages_event_cb);

	return tile;
}

void pages_loop()
{

}

bool pages_stop()
{
	return true;
}

gui_task_t gui_pages =
{
	pages_init,
	pages_loop,
	pages_stop
};
