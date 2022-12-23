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
    shortcut_get_name_cb_t cb;
);


void shortcut_set_slot(shortcut_get_name_cb_t cb, char * title, char * actual)
{
    local->cb = cb;
    uint16_t index = 0;
    gui_list_set_title(gui.list.list, title);
    for (uint16_t i = 0; i < shortcuts_get_number(); i++)
    {
        char icon[SHORTCUT_ICON_LEN];
        char text[SHORTCUT_LABEL_LEN];

        bool show = shortcut_actions[i].icon(icon, text);

        if (show)
        {
            char label[SHORTCUT_ICON_LEN + SHORTCUT_LABEL_LEN + 2];
            snprintf(label, sizeof(label), "%s %s", icon, text);
            lv_obj_t * e = gui_list_text_add_entry(gui.list.list, label, 0);

            if (strcmp(actual, shortcut_actions[i].name) == 0)
                gui_focus_child(e, NULL);

            index++;
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
                        local->cb((char *)shortcut_actions[i].name);
                        gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_BOTTOM);

                        break;
                    }
                }

            }
        }
    }

    return false;
}

lv_obj_t * shortcuts_init(lv_obj_t * parent)
{
    help_set_base("Shortcuts");

    lv_obj_t * list = gui_list_create(parent, "", NULL, shortcuts_cb);
    return list;
}
