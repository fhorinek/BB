/*
 * shortcuts.c
 *
 *  Created on: 7. 10. 2022
 *      Author: horinek
 */


#include "shortcuts.h"
#include "actions.h"
#include "gui/gui_list.h"

#include "gui/tasks/page/pages.h"

REGISTER_TASK_I(shortcuts,
    cfg_entry_t * slot;
);


void shortcut_set_slot(uint8_t slot)
{
    if (slot == 0)
        local->slot = &profile.ui.shortcut_left;
    else
        local->slot = &profile.ui.shortcut_right;

    gui_list_set_title(gui.list.list, (slot == 0) ? "Assign left shortcut" : "Assign right shortcut");
    for (uint16_t i = 0; i < shortcuts_get_number(); i++)
    {
        char icon[SHORTCUT_ICON_LEN];
        char text[SHORTCUT_LABEL_LEN];

        bool show = shortcut_actions[i].icon(icon, text);

        if (show)
        {
            char label[SHORTCUT_ICON_LEN + SHORTCUT_LABEL_LEN + 2];
            snprintf(label, sizeof(label), "%s %s", icon, text);
            gui_list_text_add_entry(gui.list.list, label);
        }
    }
}

static bool shortcuts_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
    if (event == LV_EVENT_CANCEL)
    {
        gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_BOTTOM);
    }

    if (event == LV_EVENT_PRESSED)
    {
        lv_obj_t * obj = gui_list_get_entry(index);
        if (obj != NULL)
        {
            const char * name = gui_list_text_get_value(obj);

            for (uint16_t i = 0; i < shortcuts_get_number(); i++)
            {
                char icon[SHORTCUT_ICON_LEN];
                char text[SHORTCUT_LABEL_LEN];

                bool show = shortcut_actions[i].icon(icon, text);

                if (show)
                {
                    char label[SHORTCUT_ICON_LEN + SHORTCUT_LABEL_LEN + 2];
                    snprintf(label, sizeof(label), "%s %s", icon, text);
                    if (strcmp(name, label) == 0)
                    {
                        config_set_text(local->slot, (char *)shortcut_actions[i].name);
                        break;
                    }
                }

            }
        }

        gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_BOTTOM);
    }

    return false;
}

lv_obj_t * shortcuts_init(lv_obj_t * parent)
{
    lv_obj_t * list = gui_list_create(parent, "", NULL, shortcuts_cb);
    return list;
}
