/*
 * actions.c
 *
 *  Created on: 7. 10. 2022
 *      Author: horinek
 */


#include "actions.h"
#include "gui/gui.h"

#include "shortcuts.h"


static void shrt_add_left_action()
{
    gui_switch_task(&gui_shortcuts, LV_SCR_LOAD_ANIM_MOVE_TOP);
    shortcut_set_slot(0);
}

static bool shrt_add_left_icon(char * icon, char * label)
{
    strcpy(icon, LV_SYMBOL_PLUS);
    strcpy(label, "add left shortcut");
    return false;
}

static void shrt_add_right_action()
{
    gui_switch_task(&gui_shortcuts, LV_SCR_LOAD_ANIM_MOVE_TOP);
    shortcut_set_slot(1);
}

static bool shrt_add_right_icon(char * icon, char * label)
{
    strcpy(icon, LV_SYMBOL_PLUS);
    strcpy(label, "add right shortcut");
    return false;
}


static void shrt_no_action_action()
{

}

static bool shrt_no_action_icon(char * icon, char * label)
{
    strcpy(icon, "");
    strcpy(label, "No action");
    return true;
}



shortcut_item_t shortcut_actions[] = {
    REGISTER_ACTION(add_left),
    REGISTER_ACTION(add_right),
    REGISTER_ACTION(no_action),
};

uint16_t shortcuts_get_number()
{
    return sizeof(shortcut_actions) / sizeof(shortcut_item_t);
}

shortcut_item_t * shortcuts_get_from_name(char * name)
{
    for (uint16_t i = 0; i < shortcuts_get_number(); i++)
    {
        if (strcmp(shortcut_actions[i].name, name) == 0)
            return &shortcut_actions[i];
    }

    return NULL;
}
