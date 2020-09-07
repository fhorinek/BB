/*
 * keyboard.c
 *
 *  Created on: Aug 27, 2020
 *      Author: horinek
 */


#include "keyboard.h"


void keyboard_event_cb(lv_obj_t * obj, lv_event_t event)
{
	DBG("%u", event);
	keyboard_show(obj);
}

void keyboard_create()
{
	gui.keyboard = lv_keyboard_create(lv_layer_sys(), NULL);
	lv_obj_set_size(gui.keyboard, LV_HOR_RES, GUI_KEYBOARD_SIZE);
	lv_obj_set_pos(gui.keyboard, LV_VER_RES, GUI_KEYBOARD_SIZE);
}

void keyboard_anim_cb(void * obj, lv_anim_value_t val)
{
	lv_obj_set_y(gui.keyboard, LV_VER_RES - val);
	lv_obj_set_height(lv_scr_act(), LV_VER_RES - val);
}

void keyboard_show(lv_obj_t * area)
{
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_values(&a, GUI_KEYBOARD_SIZE, 0);
	lv_anim_set_exec_cb(&a, keyboard_anim_cb);
	lv_anim_start(&a);

	lv_group_add_obj(gui.input.nav, gui.keyboard);
	lv_group_focus_obj(gui.keyboard);
	lv_group_set_editing(gui.input.nav, true);

	lv_keyboard_set_cursor_manage(gui.keyboard, true);
	lv_keyboard_set_textarea(gui.keyboard, area);
}

void keyboard_hide()
{
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_values(&a, GUI_KEYBOARD_SIZE, 0);
	lv_anim_set_exec_cb(&a, keyboard_anim_cb);
	lv_anim_start(&a);

	lv_group_remove_obj(gui.keyboard);
	lv_group_set_editing(gui.input.nav, false);
}
