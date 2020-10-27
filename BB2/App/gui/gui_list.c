/*
 * gui_list.cc
 *
 *  Created on: May 18, 2020
 *      Author: horinek
 */
#include "gui_list.h"
#include "keyboard.h"

lv_obj_t * gui_list_get_entry(uint8_t index)
{
	lv_obj_t * child = NULL;
	while(1)
	{
	    child = lv_obj_get_child_back(gui.list.object, child);

	    if (index == 0)
	    	return child;

	    index--;

	    if (child == NULL)
	    	return NULL;
	}
}

void gui_list_event_cb(lv_obj_t * obj, lv_event_t event)
{

	if (gui.list.callback == NULL)
		return;

	//get top parent
	lv_obj_t * top = obj;

	while (top->parent != gui.list.object)
	{
		top = top->parent;
		if (top == NULL)
			return;
	}

	uint8_t index = 0;

	lv_obj_t * child = NULL;
	while (1)
	{
		child = lv_obj_get_child_back(gui.list.object, child);
		if (child == top)
			break;

		if (child == NULL)
			return;

		index++;
	}

	gui.list.callback(child, event, index);
}

lv_obj_t * gui_list_create(lv_obj_t * par, const char * title, gui_list_task_cb_t cb)
{
	lv_obj_t * win = lv_win_create(par, NULL);
	lv_win_set_title(win, title);
	lv_obj_set_size(win, LV_HOR_RES, LV_VER_RES - GUI_STATUSBAR_HEIGHT);

	gui_set_group_focus(lv_win_get_content(win));
	lv_win_set_layout(win, LV_LAYOUT_COLUMN_MID);

	//object that hold list entries
	gui.list.object = lv_obj_get_child(lv_win_get_content(win), NULL);
	gui.list.callback = cb;

	return win;
}

lv_obj_t * gui_list_text_add_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	lv_page_glue_obj(entry, true);


	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_group_add_obj(gui.input.nav, entry);

	lv_obj_set_event_cb(entry, gui_list_event_cb);

	return entry;
}


lv_obj_t * gui_list_slider_add_entry(lv_obj_t * list, const char * text, int16_t value_min, int16_t value_max, int16_t value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	lv_page_glue_obj(entry, true);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_obj_t * slider = lv_slider_create(entry,  NULL);
	lv_group_add_obj(gui.input.nav, slider);
	lv_obj_set_size(slider, lv_obj_get_width_fit(entry), 14);
	lv_obj_set_focus_parent(slider, true);
	lv_obj_align(slider, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -2);

	lv_slider_set_range(slider, value_min, value_max);
	lv_slider_set_value(slider, value, LV_ANIM_OFF);

	lv_obj_set_event_cb(slider, gui_list_event_cb);

	return entry;
}

int16_t gui_list_slider_get_value(lv_obj_t * obj)
{
	//slider widget is last added child
	lv_obj_t * slider = lv_obj_get_child(obj, NULL);
	return lv_slider_get_value(slider);
}

lv_obj_t * gui_list_add_dropdown_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	lv_page_glue_obj(entry, true);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_obj_t * cont = lv_cont_create(entry, NULL);
	lv_cont_set_fit2(cont, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(cont, LV_LAYOUT_COLUMN_RIGHT);
	lv_obj_set_focus_parent(cont, true);

	lv_obj_t * dropdown = lv_dropdown_create(cont,  NULL);
	lv_group_add_obj(gui.input.nav, dropdown);
	lv_obj_set_width(dropdown, 140);
	lv_obj_set_focus_parent(dropdown, true);

	lv_obj_set_event_cb(dropdown, gui_list_event_cb);

	return entry;
}

bool gui_list_switch_get_value(lv_obj_t * obj)
{
	//switch widget is last added child
	lv_obj_t * sw = lv_obj_get_child(obj, NULL);
	return lv_switch_get_state(sw);
}

lv_obj_t * gui_list_switch_add_entry(lv_obj_t * list, const char * text, bool value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_page_glue_obj(entry, true);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);
	lv_obj_align(label, entry, LV_ALIGN_IN_LEFT_MID, lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN), 0);
	lv_obj_set_auto_realign(label, true);

	lv_obj_t * sw = lv_switch_create(entry, NULL);
	lv_group_add_obj(gui.input.nav, sw);
	lv_obj_align(sw, entry, LV_ALIGN_IN_RIGHT_MID, -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN), 0);
	lv_obj_set_auto_realign(sw, true);

	lv_obj_set_focus_parent(sw, true);

	if (value)
		lv_switch_on(sw, LV_ANIM_OFF);
	else
		lv_switch_off(sw, LV_ANIM_OFF);

	lv_obj_set_event_cb(sw, gui_list_event_cb);

	return entry;
}

lv_obj_t * gui_list_checkbox_add_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	lv_page_glue_obj(entry, true);

	lv_obj_t * checkbox = lv_checkbox_create(entry,  NULL);
	lv_group_add_obj(gui.input.nav, checkbox);
	lv_checkbox_set_text(checkbox, text);
	lv_obj_set_focus_parent(checkbox, true);

	lv_obj_set_event_cb(checkbox, gui_list_event_cb);

	return entry;
}

lv_obj_t * gui_list_info_add_entry(lv_obj_t * list, const char * text, const char * value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_NONE);
	lv_page_glue_obj(entry, true);
	lv_group_add_obj(gui.input.nav, entry);

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

	return entry;
}

void gui_list_info_set_value(lv_obj_t * obj, const char * value)
{
	//switch widget is last added child
	lv_obj_t * label = lv_obj_get_child(obj, NULL);
	lv_label_set_text(label, value);
}

void gui_list_info_set_name(lv_obj_t * obj, const char * value)
{
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
	lv_group_add_obj(gui.input.nav, sw);

	lv_page_glue_obj(entry, true);

	return sw;
}

lv_obj_t * gui_list_cont_add(lv_obj_t * list, uint16_t height)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_NONE);
	lv_obj_set_height(entry, height);
	lv_page_glue_obj(entry, true);

	lv_group_add_obj(gui.input.nav, entry);
	lv_obj_set_event_cb(entry, gui_list_event_cb);

	return entry;

}

lv_obj_t * gui_list_textbox_add_entry(lv_obj_t * list, const char * text, const char * value, uint8_t max_len)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_NONE);
	lv_page_glue_obj(entry, true);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);
	lv_obj_align(label, entry, LV_ALIGN_IN_TOP_LEFT, lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN), lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN));
	lv_obj_set_auto_realign(label, true);

	lv_obj_t * textbox = lv_textarea_create(entry, NULL);
	lv_textarea_set_text(textbox, value);
	lv_textarea_set_max_length(textbox, max_len);
	lv_textarea_set_one_line(textbox, true);
	lv_textarea_set_cursor_hidden(textbox, true);
	lv_obj_align(textbox, entry, LV_ALIGN_IN_BOTTOM_RIGHT, -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN), -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN));
	lv_obj_set_auto_realign(textbox, true);
	lv_obj_set_focus_parent(textbox, true);
	lv_group_add_obj(gui.input.nav, textbox);

	uint8_t height = lv_obj_get_height(label) + lv_obj_get_height(textbox);
	height += lv_obj_get_style_pad_top(entry, LV_CONT_PART_MAIN);
	height += lv_obj_get_style_pad_bottom(entry, LV_CONT_PART_MAIN);
	height +=- 5;

	lv_obj_set_height(entry, height);

	lv_obj_set_event_cb(textbox, keyboard_event_cb);

	return entry;
}

const char * gui_list_textbox_get_value(lv_obj_t * obj)
{
	//switch widget is last added child
	lv_obj_t * label = lv_obj_get_child(obj, NULL);
	return lv_textarea_get_text(label);
}

void gui_list_textbox_set_value(lv_obj_t * obj, const char * value)
{
	//switch widget is last added child
	lv_obj_t * label = lv_obj_get_child(obj, NULL);
	lv_textarea_set_text(label, value);
}
