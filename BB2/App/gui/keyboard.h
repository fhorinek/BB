/*
 * keyboard.h
 *
 *  Created on: Aug 27, 2020
 *      Author: horinek
 */

#ifndef GUI_KEYBOARD_H_
#define GUI_KEYBOARD_H_

#include "gui.h"

void keyboard_create();
void keyboard_show(lv_obj_t * area);
void keyboard_hide();

void keyboard_event_cb(lv_obj_t * obj, lv_event_t event);

#endif /* GUI_KEYBOARD_H_ */
