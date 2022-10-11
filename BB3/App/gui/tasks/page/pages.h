/*
 * settings.h
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */

#ifndef GUI_PAGES_H_
#define GUI_PAGES_H_

#include "gui/gui.h"

DECLARE_TASK(pages);

void pages_splash_show();

void pages_lock_widget();
void pages_unlock_widget();
void pages_lock_reset();
void gui_page_set_mode(cfg_entry_t * cfg);
void gui_page_set_next(cfg_entry_t * cfg);

void pages_set_left_shrt(char * new_shrt);
void pages_set_right_shrt(char * new_shrt);

void pages_popup(char * message);

#endif /* GUI_PAGES_H_ */
