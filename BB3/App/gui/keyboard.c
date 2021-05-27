/*
 * keyboard.c
 *
 *  Created on: Aug 27, 2020
 *      Author: horinek
 */


#include "keyboard.h"

void keyboard_obj_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CANCEL)
	{
        if (gui.keyboard.area != NULL)
        {
            lv_event_send(gui.keyboard.area, LV_EVENT_LEAVE, NULL);
        }
        keyboard_hide();
	}
	else if (event == LV_EVENT_FOCUSED)
	{
	    if (gui.keyboard.area != NULL)
	    {
            if (!lv_group_get_editing(lv_obj_get_group(gui.keyboard.area)))
            {
                keyboard_hide();
            }
	    }
	}
	else if (event == LV_EVENT_APPLY)
	{
	    if (gui.keyboard.area != NULL)
	    {
	        lv_event_send(gui.keyboard.area, LV_EVENT_APPLY, NULL);
	    }
	}
	else if (event == LV_EVENT_KEY)
	{
	    uint32_t key = *((uint32_t*) lv_event_get_data());
	    if (key == LV_KEY_HOME && gui.keyboard.area != NULL)
        {
            lv_event_send(gui.keyboard.area, LV_EVENT_APPLY, NULL);
        }
	}
	else
	{
		lv_keyboard_def_event_cb(obj, event);
	}
}

void keyboard_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_FOCUSED)
	{
		if (lv_group_get_editing(gui.input.group))
		{
            keyboard_show(obj);
		}
		else
		{
            keyboard_hide();
		}
	}
	else
	{
		gui_list_event_cb(obj, event);
	}
}

void keyboard_create()
{
	gui.keyboard.obj = lv_keyboard_create(lv_layer_sys(), NULL);
	lv_obj_set_size(gui.keyboard.obj, LV_HOR_RES, GUI_KEYBOARD_SIZE);
	lv_obj_set_pos(gui.keyboard.obj, 0, LV_VER_RES);
	lv_keyboard_set_cursor_manage(gui.keyboard.obj, true);

	lv_obj_set_event_cb(gui.keyboard.obj, keyboard_obj_event_cb);

	gui.keyboard.showed = false;
}

void keyboard_anim_cb(void * obj, lv_anim_value_t val)
{
	lv_obj_set_y(gui.keyboard.obj, LV_VER_RES - val);
	lv_obj_set_height(lv_scr_act(), LV_VER_RES - val);
}

void keyboard_show(lv_obj_t * area)
{
	if (gui.keyboard.showed)
		return;

	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_values(&a, 0, GUI_KEYBOARD_SIZE);
	lv_anim_set_exec_cb(&a, keyboard_anim_cb);
	lv_anim_start(&a);

	lv_group_add_obj(lv_obj_get_group(area), gui.keyboard.obj);
	lv_group_focus_obj(gui.keyboard.obj);
	lv_obj_move_foreground(gui.keyboard.obj);
	lv_group_set_editing(lv_obj_get_group(area), true);

	lv_keyboard_set_cursor_manage(gui.keyboard.obj, true);
	lv_keyboard_set_textarea(gui.keyboard.obj, area);
	gui.keyboard.area = area;

	gui.keyboard.showed = true;
}

void keyboard_hide()
{
	if (!gui.keyboard.showed)
		return;

	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_values(&a, GUI_KEYBOARD_SIZE, 0);
	lv_anim_set_exec_cb(&a, keyboard_anim_cb);
	lv_anim_start(&a);

	lv_keyboard_set_textarea(gui.keyboard.obj, NULL);

	lv_group_remove_obj(gui.keyboard.obj);
	lv_group_set_editing(lv_obj_get_group(gui.keyboard.area), false);
	lv_group_focus_obj(gui.keyboard.area);

	gui.keyboard.area = NULL;
	gui.keyboard.showed = false;
}
