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
char * gui_list_switch_get_title(lv_obj_t * obj);

lv_obj_t * gui_list_info_add_entry(lv_obj_t * list, const char * text, char * value);
void gui_list_info_set_value(lv_obj_t * entry, char * value);
void gui_list_info_set_name(lv_obj_t * entry, char * value);
char * gui_list_info_get_value(lv_obj_t * entry);
char * gui_list_info_get_name(lv_obj_t * entry);

lv_obj_t * gui_list_slider_add_entry(lv_obj_t * list, const char * text, int16_t value_min, int16_t value_max, int16_t value);
int16_t gui_list_slider_get_value(lv_obj_t * entry);
void gui_list_slider_set_label(lv_obj_t * obj, char * text);
void gui_list_slider_set_value(lv_obj_t * obj, int16_t value);

lv_obj_t * gui_list_textbox_add_entry(lv_obj_t * list, const char * text, const char * value, uint8_t max_len);
void gui_list_textbox_set_value(lv_obj_t * obj, const char * value);
const char * gui_list_textbox_get_value(lv_obj_t * obj);
void gui_list_textbox_set_name(lv_obj_t * obj, const char * value);

lv_obj_t * gui_list_dropdown_add_entry(lv_obj_t * list, const char * text, const char * options, uint16_t selected);
uint16_t gui_list_dropdown_get_value(lv_obj_t * obj);
void gui_list_dropdown_set_value(lv_obj_t * obj, uint8_t index);

lv_obj_t * gui_list_cont_add(lv_obj_t * list, uint16_t height);

lv_obj_t * gui_list_text_add_entry(lv_obj_t * list, const char * text);
void gui_list_text_set_value(lv_obj_t * obj, char * text);
const char * gui_list_text_get_value(lv_obj_t * obj);

lv_obj_t * gui_list_note_add_entry(lv_obj_t * list, const char * text, lv_color_t color);
void gui_list_note_set_text(lv_obj_t * obj, char * text);

lv_obj_t * gui_list_spacer_add_entry(lv_obj_t * list, uint16_t height);


lv_obj_t * gui_list_get_entry(uint16_t index);

void gui_config_entry_clear();
void gui_config_entry_add(lv_obj_t * obj, cfg_entry_t * entry, void * params);
config_entry_ll_t * gui_config_entry_find(lv_obj_t * obj);

#define NEXT_TASK			(cfg_entry_t *)0
#define CUSTOM_CB			(cfg_entry_t *)1
#define SPECIAL_HANDLING	10

lv_obj_t * gui_list_auto_entry(lv_obj_t * list, char * name, cfg_entry_t * entry, void * params);
void gui_config_entry_textbox(lv_obj_t * obj, cfg_entry_t * entry, void * params);
void gui_config_entry_update(lv_obj_t * obj, cfg_entry_t * entry, void * params);
void gui_config_entry_clicked(lv_obj_t * obj, cfg_entry_t * entry, void * params);

void gui_config_config_cb(cfg_entry_t * entry);

void gui_list_store_pos(gui_task_t * task);
void gui_list_retrive_pos(gui_task_t * task);

#define LIST_NOTE_COLOR LV_COLOR_ORANGE

//last created list
extern lv_obj_t * gui_list;

#endif /* GUI_GUI_LIST_H_ */
