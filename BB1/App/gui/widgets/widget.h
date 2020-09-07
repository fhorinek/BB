/*
 * widget.h
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#ifndef GUI_WIDGETS_WIDGET_H_
#define GUI_WIDGETS_WIDGET_H_

#include "widgets.h"

void widget_title(lv_obj_t * base, widget_slot_t * slot, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void widget_update_font_size(lv_obj_t * label, lv_obj_t * area);

//declare structure for local variabile storage during widget livetime
#define LOCAL_DECLARE(A) \
		typedef struct \
		{ \
			A \
		} widget_local_vars_t; \

#define local	((widget_local_vars_t *)slot->vars)

#endif /* GUI_WIDGETS_WIDGET_H_ */
