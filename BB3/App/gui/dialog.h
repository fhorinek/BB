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

void dialog_downloads_error(uint8_t res);

void dialog_progress_spin();
void dialog_progress_set_progress(uint8_t progress);
void dialog_progress_set_subtitle(char * text);

//data for cb
void dialog_add_opt_data(void * opt_data);
void * dialog_get_opt_data();

//param for creation
void dialog_add_opt_param(void * opt_param);

void dialog_show(char * title, char * message, dialog_type_t type, gui_dialog_cb_t cb);
void dialog_close();

#endif /* GUI_DIALOG_H_ */
