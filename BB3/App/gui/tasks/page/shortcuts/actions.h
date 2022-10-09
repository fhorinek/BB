/*
 * actions.h
 *
 *  Created on: 7. 10. 2022
 *      Author: horinek
 */

#ifndef GUI_TASKS_PAGE_SHORTCUTS_ACTIONS_H_
#define GUI_TASKS_PAGE_SHORTCUTS_ACTIONS_H_

#include "common.h"

typedef void (* shortcut_action_t)();
typedef bool (* shortcut_icon_t)(char * icon, char * label);

#define SHORTCUT_ICON_LEN   4
#define SHORTCUT_LABEL_LEN  32

typedef struct
{
    const char * name;
    shortcut_action_t action;
    shortcut_icon_t icon;
} shortcut_item_t;

extern shortcut_item_t shortcut_actions[];

#define REGISTER_ACTION(name)   {#name, shrt_ ## name ##_action, shrt_ ## name ## _icon}

uint16_t shortcuts_get_number();
shortcut_item_t * shortcuts_get_from_name(char * name);

#endif /* GUI_TASKS_PAGE_SHORTCUTS_ACTIONS_H_ */
