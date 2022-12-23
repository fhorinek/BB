/*
 * dummy.h
 *
 *  Created on: 11. 10. 2022
 *      Author: horinek
 */


static bool shrt_add_left_action()
{
    gui_switch_task(&gui_shortcuts, LV_SCR_LOAD_ANIM_MOVE_TOP);
    shortcut_set_slot(pages_set_menu_left_shrt, _("Left menu shortcut"), "");

    return false;
}

static bool shrt_add_left_icon(char * icon, char * label)
{
    strcpy(icon, LV_SYMBOL_PLUS);
    strcpy(label, _("add left shortcut"));
    return false;
}

static bool shrt_add_right_action()
{
    gui_switch_task(&gui_shortcuts, LV_SCR_LOAD_ANIM_MOVE_TOP);
    shortcut_set_slot(pages_set_menu_right_shrt, _("Right menu shortcut"), "");

    return false;
}

static bool shrt_add_right_icon(char * icon, char * label)
{
    strcpy(icon, LV_SYMBOL_PLUS);
    strcpy(label, _("add right shortcut"));
    return false;
}


static bool shrt_no_action_action()
{
    return false;
}

static bool shrt_no_action_icon(char * icon, char * label)
{
    strcpy(icon, "");
    strcpy(label, _("No action"));
    return true;
}
