/*
 * widget.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#include "widget.h"



void widget_init(lv_obj_t * base, widget_slot_t * w, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	w->obj = lv_cont_create(base, NULL);
	lv_obj_set_pos(w->obj, x, y);
	lv_obj_set_size(w->obj, width, height);
	lv_cont_set_layout(w->obj, LV_LAYOUT_PRETTY_MID);

	lv_obj_t * label = lv_label_create(w->obj, NULL);
	lv_label_set_text(label, w->widget->short_name);
}

bool widget_stop(widget_slot_t * w)
{
	lv_obj_del(w->obj);
}
