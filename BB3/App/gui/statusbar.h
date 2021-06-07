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
	STATUSBAR_MSG_ERROR = 2
} statusbar_msg_type_t;

void statusbar_show();
void statusbar_hide();
void statusbar_create();

void statusbar_step();

void statusbar_add_msg(statusbar_msg_type_t type, char * text);

#endif /* GUI_STATUSBAR_H_ */
