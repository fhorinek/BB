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
void thread_gui_start(void *argument);

#endif /* GUI_GUI_THREAD_H_ */
