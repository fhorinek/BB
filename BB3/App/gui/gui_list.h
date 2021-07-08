/*
 * gui_list.h
 *
 *  Created on: May 18, 2020
 *      Author: horinek
 */

#ifndef GUI_GUI_LIST_H_
#define GUI_GUI_LIST_H_

#include "gui.h"

typedef void (* format_cb_t)(char *, float);

typedef struct
{
	format_cb_t format;
	float disp_multi;
	float step;
} gui_list_slider_options_t;

void gui_list_init();
void gui_list_event_cb(lv_obj_t * obj, lv_event_t event);

lv_obj_t * gui_list_create(lv_obj_t * par, const char * title, gui_task_t * back, gui_list_task_cb_t cb);

lv_obj_t * gui_list_switch_add_entry(lv_obj_t * list, const char * text, bool value);
bool gui_list_switch_get_value(lv_obj_t * entry);
void gui_list_switch_set_value(lv_obj_t * obj, bool val);

lv_obj_t * gui_list_info_add_entry(lv_obj_t * list, const char * text, char * value);
void gui_list_info_set_value(lv_obj_t * entry, char * value);
void gui_list_info_set_name(lv_obj_t * entry, char * value);
char * gui_list_info_get_value(lv_obj_t * entry);
char * gui_list_info_get_name(lv_obj_t * entry);

lv_obj_t * gui_list_slider_add_entry(lv_obj_t * list, const char * text, int16_t value_min, int16_t value_max, int16_t value);
int16_t gui_list_slider_get_value(lv_obj_t * entry);
void gui_list_slider_set_label(lv_obj_t * obj, char * text);

lv_obj_t * gui_list_textbox_add_entry(lv_obj_t * list, const char * text, const char * value, uint8_t max_len);
void gui_list_textbox_set_value(lv_obj_t * obj, const char * value);
const char * gui_list_textbox_get_value(lv_obj_t * obj);

lv_obj_t * gui_list_dropdown_add_entry(lv_obj_t * list, const char * text, const char * options, uint16_t selected);
uint16_t gui_list_dropdown_get_value(lv_obj_t * obj);

lv_obj_t * gui_list_cont_add(lv_obj_t * list, uint16_t height);

lv_obj_t * gui_list_text_add_entry(lv_obj_t * list, const char * text);
void gui_list_text_set_value(lv_obj_t * obj, char * text);

lv_obj_t * gui_list_checkbox_add_entry(lv_obj_t * list, const char * text);
lv_obj_t * gui_list_add_etc_entry(lv_obj_t * list, const char * text);

lv_obj_t * gui_list_get_entry(uint8_t index);

void gui_config_entry_clear();
void gui_config_entry_add(lv_obj_t * obj, cfg_entry_t * entry, void * params);
config_entry_ll_t * gui_config_entry_find(lv_obj_t * obj);

lv_obj_t * gui_config_entry_create(lv_obj_t * list, cfg_entry_t * entry, char * name, void * params);
void gui_config_entry_update(lv_obj_t * obj, cfg_entry_t * entry, void * params);
void gui_config_entry_clicked(lv_obj_t * obj, cfg_entry_t * entry, void * params);


//last created list
extern lv_obj_t * gui_list;

#endif /* GUI_GUI_LIST_H_ */
