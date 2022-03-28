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

#define FM_OFFSET_FDATE 0
#define FM_OFFSET_FTIME 2
#define FM_OFFSET_FATTRIB 4
#define FM_OFFSET_FN 5

REGISTER_TASK_IS(filemanager,
	char path[PATH_LEN];

	/**
	 * The following is a ps_malloc'ed 2-dimensional array:
	 * filenames[FM_FILE_MAX_COUNT][PATH_LEN].
	 *
	 * Each filename starts with (unmodified taken from FILINFO):
	 *
	 *   WORD fdate;      // Modified date
	 *   WORD ftime;      // Modified time
	 *   BYTE fattrib;    // File attribute
	 *
	 * followed by the filename characters (to allow sorting).
	 * The filename can be found at offset FM_OFFSET_FN.
	 */
	unsigned char (*filenames)[PATH_LEN];

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
		//"local" variable will belong to new task now
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
		unsigned char * file = local->filenames[index];

		char new_path[PATH_LEN];

		snprintf(new_path, sizeof(new_path), "%s/%s", local->path, file + FM_OFFSET_FN);
		DBG("index=%d %s", index, new_path);

		if (file[FM_OFFSET_FATTRIB] & AM_DIR)
		{
			//we are switching to the same task
			//"local" variable will belong to new task now
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

		unsigned char * file = local->filenames[index];

		char new_path[PATH_LEN];

		snprintf(new_path, sizeof(new_path), "%s/%s", local->path, file + FM_OFFSET_FN);

        if (local->cb != NULL && local->flags & FM_FLAG_FOCUS)
        {
            if ( file[FM_OFFSET_FATTRIB] & AM_DIR )
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
			unsigned char * file = local->filenames[index];

			strncpy(filemanager_active_fname,  file + FM_OFFSET_FN, sizeof(filemanager_active_fname) - 1);

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
	local->filenames = ps_malloc(FM_FILE_MAX_COUNT*PATH_LEN);
	DBG("ps_malloc = %p", local->filenames);

	ctx_set_cb(filemanager_ctx_cb);

    return local->list;
}

static void filemanager_stop()
{
	DBG("ps_free(%p)", local->filenames);

	ps_free(local->filenames);
	local->filenames = NULL;
}

static int fm_sort_name(char *a, char *b)
{
	// Todo: Take FATTRIB into account to place directories first
	return strcmp(a + FM_OFFSET_FN, b + FM_OFFSET_FN);
}

static int fm_sort_date(WORD *a, WORD *b)
{
    if (a[0] < b[0])
        return -1;
    if (a[0] > b[0])
        return +1;
    if (a[1] < b[1])
        return -1;
    if (a[1] > b[1])
        return +1;
    return 0;
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
        while (cnt < FM_FILE_MAX_COUNT)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
            	break;

            //hide system files
            if (fno.fname[0] == '.')
                continue;

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
            }
            else
            {
                if (local->flags & FM_FLAG_HIDE_FILE)
                    continue;
            }

            local->filenames[cnt][0] = (fno.fdate >> 8) & 0xff;
            local->filenames[cnt][1] = (fno.fdate >> 0) & 0xff;
            local->filenames[cnt][2] = (fno.ftime >> 8) & 0xff;
            local->filenames[cnt][3] = (fno.ftime >> 0) & 0xff;
            local->filenames[cnt][4] = fno.fattrib;
            strcpy(&local->filenames[cnt][5], fno.fname);

            cnt++;
        }
        f_closedir(&dir);

        if (cnt == 0)
        {
            gui_list_note_add_entry(local->list, "Nothing to show", LIST_NOTE_COLOR);
            gui_set_dummy_event_cb(local->list, filemanager_dummy_cb);
        }
        else
        {
            if (local->flags & FM_FLAG_SORT_NAME)
            {
                qsort(local->filenames, cnt, PATH_LEN, fm_sort_name);
            }
            if (local->flags & FM_FLAG_SORT_DATE)
            {
                qsort(local->filenames, cnt, PATH_LEN, fm_sort_date);
            }
            for ( int i = 0; i < cnt; i++ )
            {
                char name[PATH_LEN];
                if (local->filenames[i][FM_OFFSET_FATTRIB] & AM_DIR)
                {
                    snprintf(name, sizeof(name), LV_SYMBOL_DIRECTORY " %s", &local->filenames[i][FM_OFFSET_FN]);
                }
                else
                {
                    if (local->flags & FM_FLAG_SHOW_EXT)
                    {
                        strcpy(name, &local->filenames[i][FM_OFFSET_FN]);
                    }
                    else
                    {
                        filemanager_get_filename_no_ext(name, &local->filenames[i][FM_OFFSET_FN]);
                    }
                }
                gui_list_text_add_entry(local->list, name);
            }
        }

	}
}

