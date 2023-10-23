/*
 * shortcuts.h
 *
 *  Created on: 7. 10. 2022
 *      Author: horinek
 */

#ifndef GUI_TASKS_PAGE_SHORTCUTS_SHORTCUTS_H_
#define GUI_TASKS_PAGE_SHORTCUTS_SHORTCUTS_H_

#include "gui/gui.h"

typedef void (*shortcut_set_name_cb_t)(char * name, char * page_name);

DECLARE_TASK(shortcuts);
void shortcut_set_slot(shortcut_set_name_cb_t cb, char * title, char * actual, char * page_name);

#endif /* GUI_TASKS_PAGE_SHORTCUTS_SHORTCUTS_H_ */
