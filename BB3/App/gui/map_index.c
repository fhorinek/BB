/*
 * map_index.c
 *
 *  Created on: 23. 3. 2023
 *      Author: horinek
 */

#include "map_index.h"

#include "gui/tasks/filemanager.h"
#include "gui/tasks/menu/map.h"
#include "gui_list.h"

static bool index_change = false;

bool map_index_fm_cb(uint8_t event, char * path)
{
    char name[PATH_LEN];

    if (event == FM_CB_INIT)
    {
        if (filemanager_get_current_level() == 0)
            filemanager_set_title(_("Map management"));
        else
        {
            filemanager_get_filename(name, path);
            filemanager_set_title(name);
        }

        return true;
    }

    if (event == FM_CB_CUSTOM)
    {
        filemanager_get_filename_no_ext(name, path);

        char file[32];
        snprintf(file, sizeof(file), "%s/%s", filemanager_get_title(), name);

        char * box_state = db_exists(PATH_MAP_SELECTED, file) ? MD_CHECKBOX_ON : MD_CHECKBOX_OFF;

        if (name[2] == '_')
            name[2] = ' ';

        char tmp[PATH_LEN];
        snprintf(tmp, PATH_LEN, "%s %s", box_state, name);

        gui_list_text_add_entry(gui.list.list, tmp, 0);
    }

    if (event == FM_CB_SELECT)
    {
        filemanager_get_filename_no_ext(name, path);

        char * box_state;

        char file[32];
        snprintf(file, sizeof(file), "%s/%s", filemanager_get_title(), name);

        if (db_exists(PATH_MAP_SELECTED, file))
        {
            db_delete(PATH_MAP_SELECTED, file);
            box_state = MD_CHECKBOX_OFF;
            //trigger index rebuild
            index_change = true;
        }
        else
        {
            db_insert(PATH_MAP_SELECTED, file, "");
            box_state = MD_CHECKBOX_ON;
            //trigger index rebuild
            index_change = true;
        }

        if (name[2] == '_')
            name[2] = ' ';

        char tmp[PATH_LEN];
        snprintf(tmp, PATH_LEN, "%s %s", box_state, name);

        gui_list_note_set_text(fm_selected_obj, tmp);

        return false;
    }

    if (event == FM_CB_BACK)
    {
        if (index_change)
            map_index_rebuild();
    }

    return true;
}

void map_index_show()
{
    index_change = false;
    gui_switch_task(&gui_filemanager, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    filemanager_open(PATH_MAP_LISTS, 0, &gui_map, FM_FLAG_SORT_NAME | FM_FLAG_CUSTOM, map_index_fm_cb);
}
