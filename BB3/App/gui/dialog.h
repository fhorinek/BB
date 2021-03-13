/*
 * dialog.h
 *
 *  Created on: Jan 13, 2021
 *      Author: horinek
 */

#ifndef GUI_DIALOG_H_
#define GUI_DIALOG_H_

#include "common.h"
#include "gui.h"

void dialog_show(char * title, char * message, dialog_type_t type, gui_dialog_cb_t cb);

#endif /* GUI_DIALOG_H_ */
