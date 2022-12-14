/*
 * statusbar.h
 *
 *  Created on: Aug 17, 2020
 *      Author: horinek
 */

#ifndef GUI_STATUSBAR_H_
#define GUI_STATUSBAR_H_

#include "gui.h"

typedef enum
{
	STATUSBAR_MSG_INFO = 0,
	STATUSBAR_MSG_WARN = 1,
	STATUSBAR_MSG_ERROR = 2,
	STATUSBAR_MSG_PROGRESS = 3
} statusbar_msg_type_t;

#define     I_MASK      0b00011111
#define     I_HIDE      0       //hide icon
#define     I_SHOW      1       //show icon (white)
#define     I_GRAY      2       //show icon (gray)
#define     I_YELLOW    3       //show icon (yellow)
#define     I_RED       4       //show icon (red)
#define     I_GREEN     5       //show icon (green)

#define     I_BLINK     0b10000000 //change between black and set color T = 2000ms
#define     I_FAST      0b01000000 //T = 500ms
#define     I_ALT       0b00100000 //alternate between gray and set color

void statusbar_show();
void statusbar_hide();
void statusbar_create();

void statusbar_step();

lv_obj_t * statusbar_msg_add(statusbar_msg_type_t type, char * text);
void statusbar_msg_update_progress(lv_obj_t * msg, uint8_t val);
void statusbar_msg_close(lv_obj_t * msg);
void statusbar_set_icon(uint8_t index, uint8_t state);

#endif /* GUI_STATUSBAR_H_ */
