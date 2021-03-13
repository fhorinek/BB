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
#include "config/config.h"

#define WIDGET_VAL_MIN_W    80
#define WIDGET_VAL_MIN_H    64

void widget_create_base(lv_obj_t * base, widget_slot_t * slot);
lv_obj_t * widget_add_title(lv_obj_t * base, widget_slot_t * slot, char * title);
lv_obj_t * widget_add_value(lv_obj_t * base, widget_slot_t * slot, char * unit, lv_obj_t ** unit_obj);
void widget_update_font_size(lv_obj_t * label, lv_obj_t * area);

#define local	((widget_local_vars_t *)slot->vars)

#endif /* GUI_WIDGETS_WIDGET_H_ */
