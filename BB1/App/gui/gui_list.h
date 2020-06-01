/*
 * gui_list.h
 *
 *  Created on: May 18, 2020
 *      Author: horinek
 */

#ifndef GUI_GUI_LIST_H_
#define GUI_GUI_LIST_H_

#include "gui.h"


typedef void (* gui_list_task_cb_t)(lv_obj_t *, lv_event_t, uint8_t);

void gui_list_init();

lv_obj_t * gui_list_create(lv_obj_t * par, const char * title, gui_list_task_cb_t cb);

lv_obj_t * gui_list_switch_add_entry(lv_obj_t * list, const char * text, bool value);
bool gui_list_switch_get_value(uint8_t index);


lv_obj_t * gui_list_info_add_entry(lv_obj_t * list, const char * text, const char * value);
void gui_list_info_set_value(uint8_t index, const char * value);
void gui_list_info_set_name(uint8_t index, const char * value);

lv_obj_t * gui_list_add_text_entry(lv_obj_t * list, const char * text);
lv_obj_t * gui_list_add_slider_entry(lv_obj_t * list, const char * text);
lv_obj_t * gui_list_add_dropdown_entry(lv_obj_t * list, const char * text);
lv_obj_t * gui_list_add_checkbox_entry(lv_obj_t * list, const char * text);
lv_obj_t * gui_list_add_etc_entry(lv_obj_t * list, const char * text);

lv_obj_t * gui_list_get_entry(uint8_t index);

//last created list
extern lv_obj_t * gui_list;

#endif /* GUI_GUI_LIST_H_ */
