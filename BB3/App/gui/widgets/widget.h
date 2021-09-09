/*
 * widget.h
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#ifndef GUI_WIDGETS_WIDGET_H_
#define GUI_WIDGETS_WIDGET_H_

#include "widgets.h"
#include "fc/fc.h"
#include "etc/format.h"

#define WIDGET_VAL_MIN_W    80
#define WIDGET_VAL_MIN_H    64

#define WIDGET_ARROW_POINTS 5

void widget_create_base(lv_obj_t * base, widget_slot_t * slot);
lv_obj_t * widget_add_title(lv_obj_t * base, widget_slot_t * slot, char * title);
lv_obj_t * widget_add_value(lv_obj_t * base, widget_slot_t * slot, char * unit, lv_obj_t ** unit_obj);
void widget_update_font_size(lv_obj_t * label, lv_obj_t * area);

void widget_arrow_rotate(lv_obj_t * arrow, lv_point_t * points, int16_t angle);
lv_obj_t * widget_add_arrow(lv_obj_t * base, widget_slot_t * slot, lv_point_t * points, char * unit, lv_obj_t ** unit_obj);

lv_obj_t * widget_create_edit_overlay(char * title, char * message);
void widget_destroy_edit_overlay(lv_obj_t * base);
lv_obj_t * widget_edit_overlay_get_base(lv_obj_t * edit);
void widget_reset_edit_overlay_timer();

#define local	((widget_local_vars_t *)slot->vars)

#endif /* GUI_WIDGETS_WIDGET_H_ */
