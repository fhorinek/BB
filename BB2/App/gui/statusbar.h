/*
 * statusbar.h
 *
 *  Created on: Aug 17, 2020
 *      Author: horinek
 */

#ifndef GUI_STATUSBAR_H_
#define GUI_STATUSBAR_H_

#include "gui.h"

#define STATUSBAR_MSG_INFO	0
#define STATUSBAR_MSG_WARN	1
#define STATUSBAR_MSG_ERROR	2

void statusbar_show();
void statusbar_hide();
void statusbar_create();

void statusbar_step();

void statusbar_add_msg(uint8_t type, char * text);

#endif /* GUI_STATUSBAR_H_ */
