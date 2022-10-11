/*
 * shortcuts.h
 *
 *  Created on: 7. 10. 2022
 *      Author: horinek
 */

#ifndef GUI_TASKS_PAGE_SHORTCUTS_SHORTCUTS_H_
#define GUI_TASKS_PAGE_SHORTCUTS_SHORTCUTS_H_

#include "gui/gui.h"

typedef void (*shortcut_get_name_cb_t)(char * name);

DECLARE_TASK(shortcuts);
void shortcut_set_slot(shortcut_get_name_cb_t cb, char * title, char * actual);

#endif /* GUI_TASKS_PAGE_SHORTCUTS_SHORTCUTS_H_ */
