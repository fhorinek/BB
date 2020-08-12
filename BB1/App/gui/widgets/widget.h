/*
 * widget.h
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#ifndef GUI_WIDGETS_WIDGET_H_
#define GUI_WIDGETS_WIDGET_H_

#include "widgets.h"

void widget_init(lv_obj_t * base, widget_slot_t * w, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
bool widget_stop(widget_slot_t * w);

#endif /* GUI_WIDGETS_WIDGET_H_ */
