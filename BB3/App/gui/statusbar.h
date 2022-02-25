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

#define     I_MASK      0b00111111
#define     I_HIDE      0
#define     I_SHOW      1
#define     I_GRAY      2
#define     I_YELLOW    3
#define     I_RED       4
#define     I_GREEN     5

#define     I_BLINK     0b10000000
#define     I_FAST      0b01000000

void statusbar_show();
void statusbar_hide();
void statusbar_create();

void statusbar_step();

lv_obj_t * statusbar_msg_add(statusbar_msg_type_t type, char * text);
void statusbar_msg_update_progress(lv_obj_t * msg, uint8_t val);
void statusbar_msg_close(lv_obj_t * msg);
void statusbar_set_icon(uint8_t index, uint8_t state);

#endif /* GUI_STATUSBAR_H_ */
