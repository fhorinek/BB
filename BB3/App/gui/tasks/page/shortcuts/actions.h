/*
 * actions.h
 *
 *  Created on: 7. 10. 2022
 *      Author: horinek
 */

#ifndef GUI_TASKS_PAGE_SHORTCUTS_ACTIONS_H_
#define GUI_TASKS_PAGE_SHORTCUTS_ACTIONS_H_

#include "common.h"
#include "gui/gui.h"

typedef bool (* shortcut_action_t)();
typedef bool (* shortcut_icon_t)(char * icon, char * label);

#define SHORTCUT_ICON_LEN   6
#define SHORTCUT_LABEL_LEN  32

typedef struct
{
    const char * name;
    shortcut_action_t action;
    shortcut_icon_t icon;
} shortcut_item_t;

extern shortcut_item_t shortcut_actions[];

#define ACTION_ADD_LEFT         (&shortcut_actions[0])
#define ACTION_ADD_RIGHT        (&shortcut_actions[1])
#define ACTION_NO_ACTION        (&shortcut_actions[2])


#define REGISTER_ACTION(name)   {#name, shrt_ ## name ##_action, shrt_ ## name ## _icon}

uint16_t shortcuts_get_number();
shortcut_item_t * shortcuts_get_from_name(char * name);
void shortcut_update_icon(lv_obj_t * label, shortcut_item_t * shrt);

#endif /* GUI_TASKS_PAGE_SHORTCUTS_ACTIONS_H_ */
