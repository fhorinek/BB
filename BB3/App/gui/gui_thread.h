/*
 * gui_thread.h
 *
 *  Created on: Jan 29, 2021
 *      Author: horinek
 */

#ifndef GUI_GUI_THREAD_H_
#define GUI_GUI_THREAD_H_

#include "common.h"

void gui_take_screenshot();
void gui_create_lock();
void thread_gui_start(void *argument);
void gui_set_pin(uint8_t i);

void gui_print_memory();

#endif /* GUI_GUI_THREAD_H_ */
