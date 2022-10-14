/*
 * gui_list.cc
 *
 *  Created on: May 18, 2020
 *      Author: horinek
 */
#include "gui_list.h"
#include "keyboard.h"

#include "etc/format.h"

lv_obj_t * gui_list_get_entry(uint16_t index)
{
	lv_obj_t * child = NULL;
	while(1)
	{
	    child = lv_obj_get_child_back(gui.list.content, child);

	    if (index == 0)
	    	return child;

	    index--;

	    if (child == NULL)
	    	return NULL;
	}
}

uint16_t gui_list_size()
{
    uint16_t cnt = 1;

    lv_obj_t * child = NULL;
    while (1)
    {
        child = lv_obj_get_child_back(gui.list.content, child);

        if (child == NULL)
            return cnt;

        cnt++;
    }

    return cnt;
}

uint16_t gui_list_index(lv_obj_t * obj)
{
	//get top parent
	lv_obj_t * top = obj;

	while (top->parent != gui.list.content)
	{
		top = top->parent;
		if (top == NULL)
			return GUI_LIST_NO_LAST_POS;
	}

	uint16_t index = 0;

	lv_obj_t * child = NULL;
	while (1)
	{
		child = lv_obj_get_child_back(gui.list.content, child);
		if (child == top)
			break;

		if (child == NULL)
			return GUI_LIST_NO_LAST_POS;

		index++;
	}

	return index;
}

void gui_list_event_cb(lv_obj_t * obj, lv_event_t event)
{
	uint16_t index = gui_list_index(obj);
	if (index == GUI_LIST_NO_LAST_POS)
		return;

	lv_obj_t * child = gui_list_get_entry(index);

	//call cb
	bool default_handler = true;

	gui_task_t * back_task = gui.list.back;

	if (gui.list.callback != NULL)
		default_handler = gui.list.callback(child, event, index);

	if (default_handler)
	{
		//update config entry
		config_entry_ll_t * entry = gui_config_entry_find(child);
		if (entry != NULL)
		{
			if (entry->entry == CUSTOM_CB)
			{
				bool cont = ((gui_list_task_cb_t)entry->params)(child, event, index);
				if (!cont)
					return;
			}

			if (event == LV_EVENT_LEAVE)
                gui_config_entry_textbox_cancel(child, entry->entry, entry->params);

			if (event == LV_EVENT_APPLY)
				gui_config_entry_textbox(child, entry->entry, entry->params);

			if (event == LV_EVENT_VALUE_CHANGED)
				gui_config_entry_update(child, entry->entry, entry->params);

			if (event == LV_EVENT_CLICKED)
				gui_config_entry_clicked(child, entry->entry, entry->params);
		}

		//go back
		if (event == LV_EVENT_CANCEL && back_task != NULL)
		{
			gui_switch_task(back_task, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
		}
	}
}

void gui_list_set_pos(gui_task_t * task, uint16_t pos)
{
	task->last_menu_pos = pos;
}

void gui_list_store_pos(gui_task_t * task)
{
    task->last_menu_pos = GUI_LIST_NO_LAST_POS;

    if (gui.list.content != NULL)
    {
        lv_obj_t * focused = lv_group_get_focused(gui.input.group);
        if (focused != NULL)
        {
            task->last_menu_pos = gui_list_index(focused);
        }
    }
}

bool gui_focus_child(lv_obj_t * parent, lv_obj_t * child)
{
    if (lv_obj_get_group(parent) != NULL)
    {
        lv_group_focus_obj(parent);
        return true;
    }
    else
    {
        if (child == NULL)
            child = lv_obj_get_child(parent, child);
        if (child == NULL)
            return false;
        if (gui_focus_child(child, NULL))
        {
            return true;
        }
    }
    return false;
}

void gui_list_retrive_pos(gui_task_t * task)
{
	if (gui.list.content == NULL)
		return;

	if (task->last_menu_pos == GUI_LIST_NO_LAST_POS)
		return;

	lv_obj_t * obj = gui_list_get_entry(task->last_menu_pos);
	if (obj != NULL)
	{
	    gui_focus_child(obj, NULL);
	}
}

lv_obj_t * gui_list_create(lv_obj_t * par, const char * title, gui_task_t * back, gui_list_task_cb_t cb)
{
	//clear the list
	gui_config_entry_clear();

	lv_obj_t * win = lv_win_create(par, NULL);
	lv_win_set_title(win, title);
	lv_obj_set_size(win, LV_HOR_RES, LV_VER_RES - GUI_STATUSBAR_HEIGHT);

	gui_set_group_focus(lv_win_get_content(win));
	lv_win_set_layout(win, LV_LAYOUT_PRETTY_MID);

	//object that hold list entries
	gui.list.list = win;
	gui.list.content = lv_obj_get_child(lv_win_get_content(win), NULL);
	gui.list.callback = cb;
	gui.list.back = back;

	return win;
}

void gui_list_set_title(lv_obj_t * list, const char * title)
{
	lv_win_set_title(list, title);
}

lv_obj_t * gui_list_text_add_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	//lv_page_glue_obj(entry, true);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_obj_set_event_cb(entry, gui_list_event_cb);
    lv_group_add_obj(gui.input.group, entry);

	return entry;
}

void gui_list_text_set_value(lv_obj_t * obj, char * text)
{
    lv_obj_t * label = lv_obj_get_child(obj, NULL);
    lv_label_set_text(label, text);
}

const char * gui_list_text_get_value(lv_obj_t * obj)
{
    lv_obj_t * label = lv_obj_get_child(obj, NULL);
    return lv_label_get_text(label);
}

lv_obj_t * gui_list_slider_add_entry(lv_obj_t * list, const char * text, int16_t value_min, int16_t value_max, int16_t value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_PRETTY_MID);
	//lv_page_glue_obj(entry, true);

	uint16_t w = lv_obj_get_width_fit(entry);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);
	lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL_CIRC);
	lv_obj_set_width(label, w);

	lv_obj_t * slider = lv_slider_create(entry,  NULL);
	lv_obj_set_size(slider, w / 2, 14);
	lv_obj_set_focus_parent(slider, true);

	lv_slider_set_range(slider, value_min, value_max);
	lv_slider_set_value(slider, value, LV_ANIM_OFF);
	lv_slider_set_type(slider, LV_SLIDER_TYPE_SYMMETRICAL);

	lv_obj_t * val = lv_label_create(entry, NULL);
	lv_label_set_text(val, "");
	lv_label_set_align(val, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_size(val, w / 2, 14);

	lv_obj_set_event_cb(slider, gui_list_event_cb);
    lv_group_add_obj(gui.input.group, slider);

	return entry;
}

int16_t gui_list_slider_get_value(lv_obj_t * obj)
{
	lv_obj_t * slider = lv_obj_get_child(obj, lv_obj_get_child(obj, NULL));
	return lv_slider_get_value(slider);
}


void gui_list_slider_set_value(lv_obj_t * obj, int16_t value)
{
	lv_obj_t * slider = lv_obj_get_child(obj, lv_obj_get_child(obj, NULL));
	lv_slider_set_value(slider, value, LV_ANIM_ON);
}

void gui_list_slider_set_label(lv_obj_t * obj, char * text)
{
	lv_obj_t * label = lv_obj_get_child(obj, NULL);
	lv_label_set_text(label, text);
}

lv_obj_t * gui_list_dropdown_add_entry(lv_obj_t * list, const char * text, const char * options, uint16_t selected)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	//lv_page_glue_obj(entry, true);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_obj_t * cont = lv_cont_create(entry, NULL);
	lv_cont_set_fit2(cont, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(cont, LV_LAYOUT_COLUMN_RIGHT);
	lv_obj_set_focus_parent(cont, true);

	lv_obj_t * dropdown = lv_dropdown_create(cont,  NULL);
	lv_dropdown_set_options(dropdown, options);
	lv_dropdown_set_selected(dropdown, selected);
	lv_obj_set_width(dropdown, 140);
	lv_obj_set_focus_parent(dropdown, true);

	lv_obj_set_event_cb(dropdown, gui_list_event_cb);
    lv_group_add_obj(gui.input.group, dropdown);

	return entry;
}

uint16_t gui_list_dropdown_get_value(lv_obj_t * obj)
{
    //dropdown widgt
    lv_obj_t * dd = lv_obj_get_child(lv_obj_get_child(obj, NULL), NULL);
    return lv_dropdown_get_selected(dd);
}

void gui_list_dropdown_set_value(lv_obj_t * obj, uint8_t index)
{
    //dropdown widgt
    lv_obj_t * dd = lv_obj_get_child(lv_obj_get_child(obj, NULL), NULL);
    lv_dropdown_set_selected(dd, index);
}

bool gui_list_switch_get_value(lv_obj_t * obj)
{
	//switch widget is last added child
	lv_obj_t * sw = lv_obj_get_child(obj, NULL);
	return lv_switch_get_state(sw);
}

void gui_list_switch_set_value(lv_obj_t * obj, bool val)
{
    //switch widget is last added child
    lv_obj_t * sw = lv_obj_get_child(obj, NULL);
    if (val)
        lv_switch_on(sw, LV_ANIM_ON);
    else
        lv_switch_off(sw, LV_ANIM_ON);
}

lv_obj_t * gui_list_switch_add_entry(lv_obj_t * list, const char * text, bool value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	//lv_page_glue_obj(entry, true);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);
	lv_obj_align(label, entry, LV_ALIGN_IN_LEFT_MID, lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN), 0);
	lv_obj_set_auto_realign(label, true);

	lv_obj_t * sw = lv_switch_create(entry, NULL);
	lv_obj_align(sw, entry, LV_ALIGN_IN_RIGHT_MID, -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN), 0);
	lv_obj_set_auto_realign(sw, true);

	lv_obj_set_focus_parent(sw, true);

	if (value)
		lv_switch_on(sw, LV_ANIM_OFF);
	else
		lv_switch_off(sw, LV_ANIM_OFF);

	lv_obj_set_event_cb(sw, gui_list_event_cb);
    lv_group_add_obj(gui.input.group, sw);

	return entry;
}

char * gui_list_switch_get_title(lv_obj_t * obj)
{
	lv_obj_t * label = lv_obj_get_child_back(obj, NULL);
	return lv_label_get_text(label);
}

/**
 * Add a single line containing two labels with text. The label with "text" is
 * left aligned and "value" is right aligned. Can be used to show some data, e.g.
 *
 *     Duration       10.2min
 *     Start            08:32
 *     End              08:42
 *
 *  @param list the list to add the element to
 *  @param text left aligned
 *  @param value right aligned
 *  @return the newly created lv_obj_t
 */ 
lv_obj_t * gui_list_text2_add_entry(lv_obj_t * list, const char * text, const char * value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	//lv_page_glue_obj(entry, true);

	lv_obj_t * label1 = lv_label_create(entry, NULL);
	lv_label_set_text(label1, text);
	lv_obj_align(label1, entry, LV_ALIGN_IN_LEFT_MID, lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN), 0);
	lv_obj_set_auto_realign(label1, true);

	lv_obj_t * label2 = lv_label_create(entry, NULL);
	lv_label_set_text(label2, value);
	lv_obj_align(label2, entry, LV_ALIGN_IN_RIGHT_MID, -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN), 0);
	lv_obj_set_auto_realign(label2, true);

	//lv_obj_set_focus_parent(sw, true);

	lv_obj_set_event_cb(entry, gui_list_event_cb);
    lv_group_add_obj(gui.input.group, entry);

	return entry;
}

lv_obj_t * gui_list_info_add_entry(lv_obj_t * list, const char * text, char * value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_NONE);
	//lv_page_glue_obj(entry, true);

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
	height += 2;

	lv_obj_set_height(entry, height);

	lv_obj_set_event_cb(entry, gui_list_event_cb);
    lv_group_add_obj(gui.input.group, entry);

	return entry;
}

void gui_list_info_set_value(lv_obj_t * obj, char * value)
{
	//switch widget is last added child
	lv_obj_t * label = lv_obj_get_child_back(obj, lv_obj_get_child_back(obj, NULL));
	lv_label_set_text(label, value);
}

void gui_list_info_set_name(lv_obj_t * obj, char * value)
{
	//switch widget is second last added child
	lv_obj_t * label = lv_obj_get_child_back(obj, NULL);
	lv_label_set_text(label, value);
}

char * gui_list_info_get_value(lv_obj_t * obj)
{
    //switch widget is last added child
    lv_obj_t * label = lv_obj_get_child_back(obj, lv_obj_get_child_back(obj, NULL));
    return lv_label_get_text(label);
}

char * gui_list_info_get_name(lv_obj_t * obj)
{
    //switch widget is second last added child
    lv_obj_t * label = lv_obj_get_child_back(obj, NULL);
    return lv_label_get_text(label);
}

lv_obj_t * gui_list_cont_add(lv_obj_t * list, uint16_t height)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_NONE);
	lv_obj_set_height(entry, height);
	//lv_page_glue_obj(entry, true);

	return entry;
}

lv_obj_t * gui_list_textbox_add_entry(lv_obj_t * list, const char * text, const char * value, uint8_t max_len)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_NONE);
	//lv_page_glue_obj(entry, true);

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

	uint8_t height = lv_obj_get_height(label) + lv_obj_get_height(textbox);
	height += lv_obj_get_style_pad_top(entry, LV_CONT_PART_MAIN);
	height += lv_obj_get_style_pad_bottom(entry, LV_CONT_PART_MAIN);
	height += 2;

	lv_obj_set_height(entry, height);

	lv_obj_set_event_cb(textbox, keyboard_event_cb);
    lv_group_add_obj(gui.input.group, textbox);

	return entry;
}

const char * gui_list_textbox_get_value(lv_obj_t * obj)
{
	//textarea widget is last added child
	lv_obj_t * textbox = lv_obj_get_child(obj, NULL);
	return lv_textarea_get_text(textbox);
}

void gui_list_textbox_set_value(lv_obj_t * obj, const char * value)
{
	//textarea widget is last added child
	lv_obj_t * textbox = lv_obj_get_child(obj, NULL);
	lv_textarea_set_text(textbox, value);
}

void gui_list_textbox_set_name(lv_obj_t * obj, const char * value)
{
    //label widget is first child
    lv_obj_t * label = lv_obj_get_child_back(obj, NULL);
    lv_label_set_text(label, value);
}


lv_obj_t * gui_list_note_add_entry(lv_obj_t * list, const char * text, lv_color_t color)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.note);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	//lv_page_glue_obj(entry, true);
	lv_obj_set_style_local_bg_color(entry, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, color);


	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	if (color.full == LV_COLOR_BLACK.full)
		lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	else
		lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

	return entry;
}


void gui_list_note_set_text(lv_obj_t * obj, char * text)
{
	lv_obj_t * label = lv_obj_get_child(obj, NULL);
	lv_label_set_text(label, text);
}

lv_obj_t * gui_list_spacer_add_entry(lv_obj_t * list, uint16_t height)
{
    lv_obj_t * entry = lv_obj_create(list, NULL);
    lv_obj_set_size(entry, 100, height);
    //lv_page_glue_obj(entry, true);

    return entry;
}



void gui_config_entry_clear()
{
	config_entry_ll_t * next = gui.list.entry_list;

	while (next != NULL)
	{
		config_entry_ll_t * actual = next;
		next = actual->next;
		free(actual);
	}

	gui.list.entry_list = NULL;
}

void gui_config_entry_add(lv_obj_t * obj, cfg_entry_t * entry, void * params)
{
	config_entry_ll_t * item = (config_entry_ll_t *) malloc(sizeof(config_entry_ll_t));
	item->obj = obj;
	item->entry = entry;
	item->params = params;
	item->next = NULL;

	if (gui.list.entry_list == NULL)
	{
		gui.list.entry_list = item;
		return;
	}

	config_entry_ll_t * last = gui.list.entry_list;
	while (last->next != NULL)
	{
		last = last->next;
	}

	last->next = item;
}

config_entry_ll_t * gui_config_entry_find(lv_obj_t * obj)
{
	config_entry_ll_t * next = gui.list.entry_list;

	while (next != NULL)
	{
		if (next->obj == obj)
			return next;

		next = next->next;
	}

	return NULL;
}

config_entry_ll_t * gui_config_entry_find_by_entry(cfg_entry_t * entry)
{
	config_entry_ll_t * next = gui.list.entry_list;

	while (next != NULL)
	{
		if (next->entry == entry)
			return next;

		next = next->next;
	}

	return NULL;
}

static gui_list_slider_options_t slider_default_format = {
	.disp_multi = 1,
	.step = 1,
	.format = format_int,
};

lv_obj_t * gui_list_auto_entry(lv_obj_t * list, char * name, cfg_entry_t * entry, void * params)
{
	lv_obj_t * obj = NULL;

	if (entry == NEXT_TASK)
	{
		obj = gui_list_text_add_entry(list, name);
	}
	else if (entry == CUSTOM_CB)
	{
		obj = gui_list_text_add_entry(list, name);
	}
	else
	{
		switch (entry->type)
		{
			case (ENTRY_BOOL):
				obj = gui_list_switch_add_entry(list, name, config_get_bool(entry));
				break;

			case (ENTRY_TEXT):
				obj = gui_list_textbox_add_entry(list, name, config_get_text(entry), config_text_max_len(entry));
				break;

			case (ENTRY_FLOAT):
			{
				if (params == NULL)
				{
					WARN("Slider parameter is missing!");
					params = &slider_default_format;
				}

				gui_list_slider_options_t * opt = (gui_list_slider_options_t *)params;
				int16_t min = config_float_min(entry) / opt->step;
				int16_t max = config_float_max(entry) / opt->step;
				obj = gui_list_slider_add_entry(list, name, min, max, config_get_float(entry) / opt->step);
				gui_config_entry_update(obj, entry, params);
				break;
			}

			case (ENTRY_INT16):
			{
				if (params == NULL)
				{
					WARN("Slider parameter is missing!");
					params = &slider_default_format;
				}


				gui_list_slider_options_t * opt = (gui_list_slider_options_t *)params;
				int16_t min = config_int_min(entry) / opt->step;
				int16_t max = config_int_max(entry) / opt->step;
				obj = gui_list_slider_add_entry(list, name, min, max, config_get_int(entry) / opt->step);
				gui_config_entry_update(obj, entry, params);
				break;
			}

			case (ENTRY_SELECT):
			{
				uint8_t cnt = config_get_select_cnt(entry);

				char * options = (char *) malloc(cnt * 32);
				options[0] = 0;
				for (uint8_t i = 0; i < cnt; i++)
				{
					char option[32];
					snprintf(option, sizeof(option), "%s\n", config_get_select_text_at_index(entry, i));
					strcat(options, option);
 				}
				//remove last \n
				options[strlen(options) - 1] = 0;

				obj = gui_list_dropdown_add_entry(list, name, options, config_get_select(entry));
				free(options);
				break;
			}

			default:
				obj = gui_list_info_add_entry(list, name, "???");
		}
	}

	//add to list
	gui_config_entry_add(obj, entry, params);

	return obj;
}

void gui_config_entry_clicked(lv_obj_t * obj, cfg_entry_t * entry, void * params)
{
	if (entry == NEXT_TASK)
	{
		gui_switch_task((gui_task_t *)params, LV_SCR_LOAD_ANIM_MOVE_LEFT);
	}
}

static cfg_entry_t * gui_update_entry_origin = NULL;

void gui_config_entry_textbox_cancel(lv_obj_t * obj, cfg_entry_t * entry, void * params)
{
    gui_update_entry_origin = entry;

    if (entry > (cfg_entry_t *)SPECIAL_HANDLING)
    {
        if (entry->type == ENTRY_TEXT)
        {
            keyboard_hide();
        }
    }

    gui_update_entry_origin = NULL;
}

void gui_config_entry_textbox(lv_obj_t * obj, cfg_entry_t * entry, void * params)
{
	gui_update_entry_origin = entry;

	if (entry > (cfg_entry_t *)SPECIAL_HANDLING)
	{
		if (entry->type == ENTRY_TEXT)
		{
			keyboard_hide();

			config_set_text(entry, (char *)gui_list_textbox_get_value(obj));
		}
	}

	gui_update_entry_origin = NULL;
}

void gui_config_entry_update(lv_obj_t * obj, cfg_entry_t * entry, void * params)
{
	gui_update_entry_origin = entry;

	if (entry > (cfg_entry_t *)SPECIAL_HANDLING)
	{
		switch (entry->type)
		{
			case (ENTRY_BOOL):
				config_set_bool(entry, gui_list_switch_get_value(obj));
				break;

			case (ENTRY_FLOAT):
			{
				gui_list_slider_options_t * opt = (gui_list_slider_options_t *)params;
				float value = gui_list_slider_get_value(obj) * opt->step;
				config_set_float(entry, value);
				char text[16];
				opt->format(text, value * opt->disp_multi);
				gui_list_slider_set_label(obj, text);
				break;
			}

			case (ENTRY_INT16):
			{
				gui_list_slider_options_t * opt = (gui_list_slider_options_t *)params;
				float value = gui_list_slider_get_value(obj) * opt->step;
				config_set_int(entry, value);
				char text[16];
				opt->format(text, value * opt->disp_multi);
				gui_list_slider_set_label(obj, text);
				break;
			}

			case (ENTRY_SELECT):
			{
				uint8_t index = gui_list_dropdown_get_value(obj);
				uint8_t val = config_get_select_at_index(entry, index);
				config_set_select(entry, val);
			}
		}
	}

	gui_update_entry_origin = NULL;
}

void gui_config_entry_refresh(lv_obj_t * obj, cfg_entry_t * entry, void * params)
{
	if (entry > (cfg_entry_t *)SPECIAL_HANDLING)
	{
		switch (entry->type)
		{
			case (ENTRY_BOOL):
				gui_list_switch_set_value(obj, config_get_bool(entry));
				break;

			case (ENTRY_TEXT):
				gui_list_textbox_set_value(obj, config_get_text(entry));
				break;

			case (ENTRY_FLOAT):
			{
				gui_list_slider_options_t * opt = (gui_list_slider_options_t *)params;
				int16_t value = config_get_float(entry) / opt->step;
				gui_list_slider_set_value(obj, value);
				char text[16];
				opt->format(text, value * opt->disp_multi);
				gui_list_slider_set_label(obj, text);
				break;
			}

			case (ENTRY_INT16):
			{
				gui_list_slider_options_t * opt = (gui_list_slider_options_t *)params;
				int16_t value = config_get_int(entry) / opt->step;
				gui_list_slider_set_value(obj, value);
				char text[16];
				opt->format(text, value * opt->disp_multi);
				gui_list_slider_set_label(obj, text);
				break;
			}

			case (ENTRY_SELECT):
			{
				uint8_t val = config_get_select_index(entry);
				gui_list_dropdown_set_value(obj, val);
			}
		}
	}

	lv_obj_invalidate(obj);
}

void gui_config_config_cb(cfg_entry_t * entry)
{
	if (entry == gui_update_entry_origin)
		return;

	config_entry_ll_t * e = gui_config_entry_find_by_entry(entry);

	if (e != NULL)
	{
	    gui_lock_acquire();
		gui_config_entry_refresh(e->obj, entry, e->params);
	    gui_lock_release();
	}
}
