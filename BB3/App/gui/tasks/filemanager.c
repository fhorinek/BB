/*
 * filemanager.c
 *
 * Created on: Feb 27, 2021
 *      Author: horinek
 *
 * The filemanager allows the user to navigate through a file directory, select files and
 * offer additional functionality in a context menu. Here is an overview on how to use it.
 * See specific functions for more detailled information.
 *
 *
 * To open a filemanager on a specific directory you first "gui_switch_task" to the gui_filemanager
 *      gui_switch_task(&gui_filemanager, (anim) ? LV_SCR_LOAD_ANIM_MOVE_LEFT : LV_SCR_LOAD_ANIM_NONE);
 * and then use "filemanager_open" to tell the filemanager what to do.
 *
 * Whenever a file gets selected, the callback function given to filemanager_open will be called.
 * This callback function receives two parameters: "event" and "path". Event is one of FM_CB_BACK and others
 * telling the callback function the reason for being called. "path" is the filename selected.
 *
 * See FM_CB_BACK and others for a description of the meaning.
 */

#define DEBUG_LEVEL	DBG_DEBUG
#include "filemanager.h"

#include "gui/gui_list.h"
#include "gui/ctx.h"

REGISTER_TASK_I(filemanager,
	char path[PATH_LEN];
	lv_obj_t * list;
	gui_task_t * back;
	filemanager_cb_t cb;

	uint8_t flags;
	uint8_t level;
);

static char filemanager_active_fname[32];

//dst should point to valid space in memory with size at least as path
bool filemanager_get_filename_no_ext(char * dst, char * path)
{
    char * start = dst;
    strcpy(dst, path);
    dst = strrchr(dst, '/');

    if (dst != NULL)
        dst++;
    else
        dst = start;

    char * dot = strrchr(dst, '.');
    if (dot == NULL)
        return false;
    *dot = 0;

    memmove(start, dst, strlen(dst) + 1);
    return true;
}

//dst should point to valid space in memory with size at least as path
bool filemanager_get_path(char * dst, char * path)
{
    strcpy(dst, path);
    dst = strrchr(dst, '/');

    if (dst == NULL)
        return false;
    *dst = 0;

    return true;
}


//dst should point to valid space in memory with size at least as path
bool filemanager_get_filename(char * dst, char * path)
{
    char * start = dst;
    strcpy(dst, path);
    dst = strrchr(dst, '/');

    if (dst != NULL)
        dst++;
    else
        dst = start;

    memmove(start, dst, strlen(dst) + 1);
    return true;
}


bool filemanager_ctx_cb(uint8_t index, lv_obj_t * obj)
{
    if (local->cb != NULL)
    {
        char new_path[PATH_LEN];
        snprintf(new_path, sizeof(new_path), "%s/%s", local->path, filemanager_active_fname);

        return local->cb(index, new_path);
    }

    return true;
}

#define DIR_ICON	 LV_SYMBOL_DIRECTORY " "

bool filemanager_back()
{
	if (local->level == 0)
	{
	    bool ret = true;
        if (local->cb != NULL)
            ret = local->cb(FM_CB_BACK, "");

        if (ret)
        	gui_switch_task(local->back, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
	}
	else
	{
		char new_path[PATH_LEN];
		strncpy(new_path, local->path, sizeof(new_path));
		char * ptr = strrchr(new_path, '/');
		if (ptr != NULL)
		{
			*ptr = 0;
		}
		else
		{
			gui_switch_task(local->back, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
			return false;
		}

		//we are switching to the same task
		//"local" variabile will belong to new task now
		gui_local_vars_t * old = gui_switch_task(&gui_filemanager, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
		filemanager_open(new_path, old->level - 1, old->back, old->flags, old->cb);
	}
	return true;
}

static void filemanager_dummy_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CANCEL)
    {
	    bool ret = true;
        if (local->cb != NULL)
            ret = local->cb(FM_CB_CANCEL, "");

        if (ret)
        	filemanager_back();
    }
}

static bool filemanager_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
	if (event == LV_EVENT_CANCEL)
	{
	    bool ret = true;
        if (local->cb != NULL)
            ret = local->cb(FM_CB_CANCEL, "");

        if (ret)
        	filemanager_back();
	}

	if (event == LV_EVENT_CLICKED)
	{
		char * fname = (char *)gui_list_text_get_value(obj);
		if (strstr(fname, DIR_ICON) != NULL)
			fname += strlen(DIR_ICON);

		char new_path[PATH_LEN];

		snprintf(new_path, sizeof(new_path), "%s/%s", local->path, fname);

		if (file_isdir(new_path))
		{
			//we are switching to the same task
			//"local" variabile will belong to new task now
			gui_local_vars_t * old = gui_switch_task(&gui_filemanager, LV_SCR_LOAD_ANIM_MOVE_LEFT);
			filemanager_open(new_path, old->level + 1, old->back, old->flags, old->cb);
		}
		else
		{
		    bool ret = true;
			if (local->cb != NULL)
				ret = local->cb(FM_CB_SELECT, new_path);

            if (ret)
                gui_switch_task(local->back, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

		}

	}

	if (event == LV_EVENT_FOCUSED)
	{
		ctx_hide();

		char * fname = (char *)gui_list_text_get_value(obj);
		if (strstr(fname, DIR_ICON) != NULL)
			fname += strlen(DIR_ICON);
		char new_path[PATH_LEN];

		snprintf(new_path, sizeof(new_path), "%s/%s", local->path, fname);

        if (local->cb != NULL && local->flags & FM_FLAG_FOCUS)
        {
            if (file_isdir(new_path))
                local->cb(FM_CB_FOCUS_DIR, new_path);
            else
                local->cb(FM_CB_FOCUS_FILE, new_path);
        }
	}

	if (event == LV_EVENT_KEY)
	{
		uint32_t key = *((uint32_t *) lv_event_get_data());
		if (key == LV_KEY_HOME && ctx_is_active())
		{
			char * fname = (char *)gui_list_text_get_value(obj);
			if (strstr(fname, DIR_ICON) != NULL)
				fname += strlen(DIR_ICON);

			strncpy(filemanager_active_fname,  fname, sizeof(filemanager_active_fname) - 1);

			ctx_open(0);
		}
	}

	return false;
}


static lv_obj_t * filemanager_init(lv_obj_t * par)
{
	local->list = gui_list_create(par, "", NULL, filemanager_cb);
	local->cb = NULL;
	local->path[0] = 0;
	local->flags = 0;

	ctx_set_cb(filemanager_ctx_cb);

    return local->list;
}

/**
 * Start a filemanager on the given path and use callback function to control everything.
 *
 * @param path the directory name to show
 * @param level the current level inside directory. Start with 0 and it wil be increased on every further directory.
 * @param back the gui_task_t to return to, if the filemanager is closed.
 * @param flags use FM_FLAG_HIDE_DIR and others combined with bitwise or. They control behaviour of filemanager.
 * @param cb the callback function. See description on top of file.
 */
void filemanager_open(char * path, uint8_t level, gui_task_t * back, uint8_t flags, filemanager_cb_t cb)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;

    local->flags = flags;

    if (level > 100)
    	DBG("too deep!");

    DBG("path '%s'; level %u; back %X; cb %X", path, level, back, cb);

    if (strlen(path) == 0)
    {
    	lv_win_set_title(local->list, "Strato");
    }
    else
    {
    	lv_win_set_title(local->list, path);
    }

	local->level = level;
	local->back = back;
	local->cb = cb;
	strncpy(local->path, path, PATH_LEN);

	res = f_opendir(&dir, path);
	if (res == FR_OK)
	{
		uint16_t cnt = 0;
        while (true)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
            	break;


            if (local->flags & FM_FLAG_FILTER)
            {
                char new_path[PATH_LEN];
                snprintf(new_path, sizeof(new_path), "%s/%s", local->path, fno.fname);

                if (!local->cb(FM_CB_FILTER, new_path))
                    continue;
            }

            if (fno.fattrib & AM_DIR)
            {
                if (local->flags & FM_FLAG_HIDE_DIR)
                    continue;

            	char name[PATH_LEN];
            	snprintf(name, sizeof(name), DIR_ICON "%s", fno.fname);
            	gui_list_text_add_entry(local->list, name);
                cnt++;
            }
            else
            {
                if (local->flags & FM_FLAG_HIDE_FILE)
                    continue;

                gui_list_text_add_entry(local->list, fno.fname);
                cnt++;
            }
        }

        if (cnt == 0)
        {
            gui_list_note_add_entry(local->list, "Nothing to show", LIST_NOTE_COLOR);
            gui_set_dummy_event_cb(local->list, filemanager_dummy_cb);
        }

        f_closedir(&dir);
	}
}

