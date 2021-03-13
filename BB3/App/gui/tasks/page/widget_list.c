/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "page_edit.h"
#include "widget_list.h"

#include "gui/gui_list.h"
#include "config/config.h"
#include "gui/widgets/widgets.h"

REGISTER_TASK_I(widget_list,
    char page_name[PAGE_NAME_LEN + 1];
    uint8_t widget_index;

    widget_slot_t prev_ws;
    lv_obj_t * prev_obj;
);

void widget_list_set_page_name(char * name)
{
    //load page from file
    strncpy(local->page_name, name, sizeof(local->page_name));
}

void widget_list_select_widget(widget_t * w, uint8_t widget_index)
{
    local->widget_index = widget_index;

    if (w != NULL)
    {
        for (uint16_t i = 0; i < number_of_widgets(); i++)
        {
            if (widgets[i] == w)
            {
                lv_group_focus_obj(gui_list_get_entry(i));
                break;
            }
        }
    }
}


void widget_list_show_widget(uint8_t index)
{
    if (local->prev_ws.widget != NULL)
    {
        widget_deinit(&local->prev_ws);
        lv_obj_del(local->prev_ws.obj);
    }

    local->prev_ws.x = 0;
    local->prev_ws.y = 0;
    local->prev_ws.w = 0;
    local->prev_ws.h = 0;

    local->prev_ws.widget = widgets[index];

    widget_init(&local->prev_ws, local->prev_obj);
}

static void widget_list_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
	{
		gui_switch_task(&gui_page_edit, LV_SCR_LOAD_ANIM_MOVE_LEFT);
		page_edit_set_page_name(local->page_name);
	}

	if (event == LV_EVENT_PRESSED)
	{
        gui_switch_task(&gui_page_edit, LV_SCR_LOAD_ANIM_MOVE_LEFT);
        page_edit_set_page_name(local->page_name);
        page_edit_modify_widget(widgets[index], local->widget_index);
	}

    if (event == LV_EVENT_FOCUSED)
    {
        widget_list_show_widget(index);
    }
}



#define WIDGET_LIST_PREV_HEIGHT 120

static lv_obj_t * widget_list_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Select widget", widget_list_cb);

	lv_obj_set_y(list, WIDGET_LIST_PREV_HEIGHT);
	lv_obj_set_height(list, LV_VER_RES - GUI_STATUSBAR_HEIGHT - WIDGET_LIST_PREV_HEIGHT);

	local->prev_obj = lv_cont_create(par, NULL);
	lv_obj_set_pos(local->prev_obj, 0, 0);
	lv_obj_set_size(local->prev_obj, LV_HOR_RES, WIDGET_LIST_PREV_HEIGHT);
	lv_cont_set_layout(local->prev_obj, LV_LAYOUT_CENTER);
    lv_obj_set_style_local_bg_color(local->prev_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_radius(local->prev_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);

	local->prev_ws.widget = NULL;

	for (uint16_t i = 0; i < number_of_widgets(); i++)
	{
	    widget_t * w = widgets[i];
        gui_list_text_add_entry(list, w->name);
	}

    widget_list_show_widget(0);

	return list;
}
