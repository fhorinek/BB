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
		keyboard_hide();
	}
	else if (event == LV_EVENT_FOCUSED)
	{
		if (!lv_group_get_editing(gui.input.nav))
		{
			keyboard_hide();
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
		if (lv_group_get_editing(gui.input.nav))
		{
				keyboard_show(obj);
		}
		else
		{
				keyboard_hide();
		}
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

	lv_group_add_obj(gui.input.nav, gui.keyboard.obj);
	lv_group_focus_obj(gui.keyboard.obj);
	lv_group_set_editing(gui.input.nav, true);

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
	lv_group_set_editing(gui.input.nav, false);
	lv_group_focus_obj(gui.keyboard.area);

	gui.keyboard.showed = false;
}
