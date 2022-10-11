/*
 * dummy.h
 *
 *  Created on: 11. 10. 2022
 *      Author: horinek
 */


static bool shrt_add_left_action()
{
    gui_switch_task(&gui_shortcuts, LV_SCR_LOAD_ANIM_MOVE_TOP);
    shortcut_set_slot(pages_set_left_shrt, "Left menu shortcut", "");

    return false;
}

static bool shrt_add_left_icon(char * icon, char * label)
{
    strcpy(icon, LV_SYMBOL_PLUS);
    strcpy(label, "add left shortcut");
    return false;
}

static bool shrt_add_right_action()
{
    gui_switch_task(&gui_shortcuts, LV_SCR_LOAD_ANIM_MOVE_TOP);
    shortcut_set_slot(pages_set_right_shrt, "Right menu shortcut", "");

    return false;
}

static bool shrt_add_right_icon(char * icon, char * label)
{
    strcpy(icon, LV_SYMBOL_PLUS);
    strcpy(label, "add right shortcut");
    return false;
}


static void shrt_no_action_action() {}
static bool shrt_no_action_icon(char * icon, char * label)
{
    strcpy(icon, "");
    strcpy(label, "No action");
    return true;
}

static bool shrt_dummy_action()
{
    dialog_show("Dummy", "Dummy action", dialog_confirm, NULL);

    return false;
}

static bool shrt_dummy_icon(char * icon, char * label)
{
    strcpy(icon, LV_SYMBOL_STOP);
    strcpy(label, "Dummy action");
    return true;
}

