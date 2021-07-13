#include "filemanager.h"

#include "gui/gui_list.h"
#include "gui/ctx.h"

REGISTER_TASK_I(filemanager,
	char path[PATH_LEN];
	lv_obj_t * list;
	gui_task_t * back;
	filemanager_cb_t cb;

	uint8_t level;
);

void filemanager_ctx_cb(uint8_t index, lv_obj_t * obj)
{

}

#define DIR_ICON	 LV_SYMBOL_DIRECTORY " "

void filemanager_back()
{
	if (local->level == 0)
	{
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
		filemanager_open(new_path, old->level - 1, old->back, old->cb);
	}

}

static bool filemanager_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
	{
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
			filemanager_open(new_path, old->level + 1, old->back, old->cb);
		}
		else
		{
			if (local->cb != NULL)
				local->cb(new_path);
		}

	}

	if (event == LV_EVENT_FOCUSED)
	{
		char * fname = (char *)gui_list_text_get_value(obj);
		if (strstr(fname, DIR_ICON) != NULL)
			fname += strlen(DIR_ICON);
		char new_path[PATH_LEN];

		snprintf(new_path, sizeof(new_path), "%s/%s", local->path, fname);

		if (file_isdir(new_path))
		{
			ctx_show();
			ctx_clear();
			ctx_add_option("asdf", 0);
			ctx_add_option("asdf", 1);
			ctx_add_option("asdf", 2);
			ctx_add_option("asdf", 3);
		}
		else
		{
			ctx_hide();
		}
	}

	if (event == LV_EVENT_KEY)
	{
		uint32_t key = *((uint32_t *) lv_event_get_data());
		if (key == LV_KEY_HOME)
			ctx_open();
	}

	return false;
}


static lv_obj_t * filemanager_init(lv_obj_t * par)
{
	local->list = gui_list_create(par, "", NULL, filemanager_cb);
	local->cb = NULL;
	local->path[0] = 0;

	ctx_set_cb(filemanager_ctx_cb);

    return local->list;
}

void filemanager_open(char * path, uint8_t level, gui_task_t * back, filemanager_cb_t cb)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;

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

            cnt++;

            if (fno.fattrib & AM_DIR)
            {
            	char name[PATH_LEN];
            	snprintf(name, sizeof(name), DIR_ICON "%s", fno.fname);
            	gui_list_text_add_entry(local->list, name);
            }
            else
            {
            	gui_list_text_add_entry(local->list, fno.fname);
            }
        }

        if (cnt == 0)
        {
        	gui_list_text_add_entry(local->list, "Folder is empty");
        }

        f_closedir(&dir);
	}
}

