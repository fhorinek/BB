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

void statusbar_show();
void statusbar_hide();
void statusbar_create();

void statusbar_step();

lv_obj_t * statusbar_msg_add(statusbar_msg_type_t type, char * text);
void statusbar_msg_update_progress(lv_obj_t * msg, uint8_t val);
void statusbar_msg_close(lv_obj_t * msg);

#endif /* GUI_STATUSBAR_H_ */
