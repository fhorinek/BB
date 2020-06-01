/*
 * gui_list.cc
 *
 *  Created on: May 18, 2020
 *      Author: horinek
 */
#include "gui_list.h"

//styles
lv_style_t style_no_border;
lv_style_t style_list_entry;

lv_obj_t * gui_list;
lv_obj_t * gui_list_obj;
gui_list_task_cb_t gui_list_task_cb;

void gui_list_init()
{
	lv_style_init(&style_no_border);
	lv_style_set_border_width(&style_no_border, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_inner(&style_no_border, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_top(&style_no_border, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_bottom(&style_no_border, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_left(&style_no_border, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_right(&style_no_border, LV_STATE_DEFAULT, 15);

	lv_style_init(&style_list_entry);
	lv_style_set_border_width(&style_list_entry, LV_STATE_DEFAULT, 1);
	lv_style_set_border_side(&style_list_entry, LV_STATE_DEFAULT, LV_BORDER_SIDE_TOP);
	lv_style_set_radius(&style_list_entry, LV_STATE_DEFAULT, 0);
	lv_style_set_margin_top(&style_list_entry, LV_STATE_DEFAULT, 0);
	lv_style_set_margin_bottom(&style_list_entry, LV_STATE_DEFAULT, 0);
	lv_style_set_margin_left(&style_list_entry, LV_STATE_DEFAULT, 0);
	lv_style_set_margin_right(&style_list_entry, LV_STATE_DEFAULT, 0);

	lv_style_set_bg_color(&style_list_entry, LV_STATE_FOCUSED, LV_COLOR_BLUE);

	lv_style_set_bg_color(&style_list_entry, LV_STATE_EDITED, LV_COLOR_RED);
}

lv_obj_t * gui_list_get_entry(uint8_t index)
{
	lv_obj_t * child = NULL;
	while(1)
	{
	    child = lv_obj_get_child_back(gui_list_obj, child);

	    if (index == 0)
	    	return child;

	    index--;

	    if (child == NULL)
	    	return NULL;
	}
}

void gui_list_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (gui_list_task_cb == NULL)
		return;

	//get top parent
	lv_obj_t * top = obj;

	while (top->parent != gui_list_obj)
	{
		top = top->parent;
		if (top == NULL)
			return;
	}

	uint8_t index = 0;

	lv_obj_t * child = NULL;
	while (1)
	{
		child = lv_obj_get_child_back(gui_list_obj, child);
		if (child == top)
			break;

		if (child == NULL)
			return;

		index++;
	}

	gui_list_task_cb(obj, event, index);
}

lv_obj_t * gui_list_create(lv_obj_t * par, const char * title, gui_list_task_cb_t cb)
{
	lv_obj_t * win = lv_win_create(par, NULL);
	lv_win_set_title(win, title);
	lv_obj_set_size(win, LV_HOR_RES, LV_VER_RES);

	gui_set_page_focus(lv_win_get_content(win));
	lv_win_set_layout(win, LV_LAYOUT_COLUMN_MID);

	//lv_obj_add_style(win, LV_WIN_PART_CONTENT_SCROLLABLE, &style_no_border);

	//object that hold list entries
	gui_list_obj = lv_obj_get_child(lv_win_get_content(win), NULL);
	gui_list = win;
	gui_list_task_cb = cb;

	return win;
}

lv_obj_t * gui_list_add_text_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	lv_page_glue_obj(entry, true);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &style_list_entry);


	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_group_add_obj(gui_group, entry);


	lv_obj_set_event_cb(entry, gui_list_event_cb);

	return entry;
}


lv_obj_t * gui_list_add_slider_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	lv_page_glue_obj(entry, true);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &style_list_entry);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_obj_t * cont = lv_cont_create(entry, NULL);
	lv_cont_set_fit2(cont, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(cont, LV_LAYOUT_COLUMN_RIGHT);
	lv_obj_add_style(cont, LV_CONT_PART_MAIN, &style_no_border);
	lv_obj_set_focus_parent(cont, true);

	lv_obj_t * slider = lv_slider_create(cont,  NULL);
	lv_group_add_obj(gui_group, slider);
	lv_obj_set_focus_parent(slider, true);

	lv_obj_set_event_cb(slider, gui_list_event_cb);

	return slider;
}

lv_obj_t * gui_list_add_dropdown_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	lv_page_glue_obj(entry, true);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &style_list_entry);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_obj_t * cont = lv_cont_create(entry, NULL);
	lv_cont_set_fit2(cont, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(cont, LV_LAYOUT_COLUMN_RIGHT);
	lv_obj_add_style(cont, LV_CONT_PART_MAIN, &style_no_border);
	lv_obj_set_focus_parent(cont, true);

	lv_obj_t * dropdown = lv_dropdown_create(cont,  NULL);
	lv_group_add_obj(gui_group, dropdown);
	lv_obj_set_width(dropdown, 140);
	lv_obj_set_focus_parent(dropdown, true);

	lv_obj_set_event_cb(dropdown, gui_list_event_cb);

	return dropdown;
}

bool gui_list_switch_get_value(uint8_t index)
{
	lv_obj_t * obj = gui_list_get_entry(index);
	//switch widget is last added child
	lv_obj_t * sw = lv_obj_get_child(obj, NULL);
	return lv_switch_get_state(sw);
}

lv_obj_t * gui_list_switch_add_entry(lv_obj_t * list, const char * text, bool value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_page_glue_obj(entry, true);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &style_list_entry);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);
	lv_obj_align(label, entry, LV_ALIGN_IN_LEFT_MID, lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN), 0);
	lv_obj_set_auto_realign(label, true);

	lv_obj_t * sw = lv_switch_create(entry, NULL);
	lv_group_add_obj(gui_group, sw);
	lv_obj_align(sw, entry, LV_ALIGN_IN_RIGHT_MID, -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN), 0);
	lv_obj_set_auto_realign(sw, true);

	lv_obj_set_focus_parent(sw, true);

	if (value)
		lv_switch_on(sw, LV_ANIM_OFF);
	else
		lv_switch_off(sw, LV_ANIM_OFF);

	lv_obj_set_event_cb(sw, gui_list_event_cb);

	return sw;
}

lv_obj_t * gui_list_add_checkbox_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	lv_page_glue_obj(entry, true);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &style_list_entry);

	lv_obj_t * checkbox = lv_checkbox_create(entry,  NULL);
	lv_group_add_obj(gui_group, checkbox);
	lv_checkbox_set_text(checkbox, text);
	lv_obj_set_focus_parent(checkbox, true);

	lv_obj_set_event_cb(checkbox, gui_list_event_cb);

	return checkbox;
}

lv_obj_t * gui_list_info_add_entry(lv_obj_t * list, const char * text, const char * value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_NONE);
	lv_page_glue_obj(entry, true);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &style_list_entry);
	lv_group_add_obj(gui_group, entry);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);
	lv_obj_align(label, entry, LV_ALIGN_IN_TOP_LEFT, lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN), lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN));
	lv_obj_set_auto_realign(label, true);

	lv_obj_t * info = lv_label_create(entry, NULL);
	lv_label_set_text(info, value);
	lv_obj_align(info, entry, LV_ALIGN_IN_BOTTOM_RIGHT, -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN), -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN));
	lv_obj_set_auto_realign(info, true);

	uint8_t height = lv_obj_get_height(label) + lv_obj_get_height(info);
	height += lv_obj_get_style_pad_top(entry, LV_CONT_PART_MAIN);
	height += lv_obj_get_style_pad_bottom(entry, LV_CONT_PART_MAIN);
	height +=- 5;

	lv_obj_set_height(entry, height);

	lv_obj_set_event_cb(entry, gui_list_event_cb);

	return info;
}

void gui_list_info_set_value(uint8_t index, const char * value)
{
	lv_obj_t * obj = gui_list_get_entry(index);
	//switch widget is last added child
	lv_obj_t * label = lv_obj_get_child(obj, NULL);
	lv_label_set_text(label, value);
}

void gui_list_info_set_name(uint8_t index, const char * value)
{
	lv_obj_t * obj = gui_list_get_entry(index);
	//switch widget is second last added child
	lv_obj_t * label = lv_obj_get_child(obj, lv_obj_get_child(obj, NULL));
	lv_label_set_text(label, value);
}


lv_obj_t * gui_list_add_etc_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_obj_t * sw = lv_spinbox_create(entry,  NULL);
	lv_group_add_obj(gui_group, sw);

	lv_page_glue_obj(entry, true);

	return sw;
}
