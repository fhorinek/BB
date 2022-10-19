/*
 * filemanager.c
 *
 * Created on: Feb 27, 2021
 *      Author: horinek
 *
 * The filemanager allows the user to navigate through a file directory, select files and
 * offer additional functionality in a context menu. Here is an overview on how to use it.
 * See specific functions for more detailed information.
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

typedef struct
{
        uint32_t date;
        uint16_t mode;
        char name[REDCONF_NAME_MAX + 1];
        uint8_t _pad[1];
} fm_record_cache_t;

REGISTER_TASK_IS(filemanager,
	char path[PATH_LEN];

    fm_record_cache_t *filenames;

	lv_obj_t * list;
	gui_task_t * back;
	filemanager_cb_t cb;

	uint8_t flags;
	uint8_t level;
	uint16_t filenames_count;
	uint32_t inode;
);

static char filemanager_active_fname[REDCONF_NAME_MAX];

#define FILEMANAGER_HISTORY     8

typedef struct {
    uint32_t used;
    uint32_t inode;
    uint16_t pos;
    uint8_t _pad[2];
} fm_history_node;

static fm_history_node filemanager_history[FILEMANAGER_HISTORY] = {0};

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

//only call in filemanager callbacks, when filemanager is active task!
uint8_t filemanager_get_current_level()
{
	ASSERT(gui.task.actual == &gui_filemanager);
	return local->level;
}

uint16_t filemanager_retrive_inode_pos(uint32_t inode)
{
    for (uint8_t i = 0; i < FILEMANAGER_HISTORY; i++)
    {
        if (filemanager_history[i].inode == inode)
        {
            return filemanager_history[i].pos;
        }
    }

    return 0;
}

void filemanager_store_inode_pos(uint32_t inode, uint16_t pos)
{
    //find inode record
    for (uint8_t i = 0; i < FILEMANAGER_HISTORY; i++)
    {
        if (filemanager_history[i].inode == inode)
        {
            filemanager_history[i].pos = pos;
            filemanager_history[i].used = HAL_GetTick();
            return;
        }
    }

    //find oldest or unused
    uint32_t oldest = 0xFFFFFFFF;
    uint8_t index = 0;
    for (uint8_t i = 0; i < FILEMANAGER_HISTORY; i++)
    {
        if (oldest > filemanager_history[i].used)
        {
            oldest = filemanager_history[i].used;
            index = i;
        }
    }

    filemanager_history[index].inode = inode;
    filemanager_history[index].pos = pos;
    filemanager_history[index].used = HAL_GetTick();
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

	//appended item, use default handler
	if (index >= local->filenames_count)
		return true;

	if (event == LV_EVENT_CLICKED)
	{
	    fm_record_cache_t * file = &local->filenames[index];

		char new_path[PATH_LEN];

		snprintf(new_path, sizeof(new_path), "%s/%s", local->path, file->name);
		DBG("index=%d %s", index, new_path);

		if (RED_S_ISDIR(file->mode))
		{
			//we are switching to the same task
			//"local" variable will belong to new task after gui_switch_task
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

		fm_record_cache_t * file = &local->filenames[index];

		char new_path[PATH_LEN];

		snprintf(new_path, sizeof(new_path), "%s/%s", local->path, file->name);

        if (local->cb != NULL && local->flags & FM_FLAG_FOCUS)
        {
            if (RED_S_ISDIR(file->mode))
                local->cb(FM_CB_FOCUS_DIR, new_path);
            else
                local->cb(FM_CB_FOCUS_FILE, new_path);
        }

        //store actual file position
        if (local->inode != 0) //only after local->inode is set at the end of the filemanager_open
        {
            lv_obj_t * focused = lv_group_get_focused(gui.input.group);
            if (focused != NULL)
            {
                filemanager_store_inode_pos(local->inode, gui_list_index(focused));
            }
        }
	}

    if (event == LV_EVENT_KEY_LONG_PRESSED)
    {
        return true;
    }

	if (event == LV_EVENT_KEY_RELEASED)
	{
		uint32_t key = *((uint32_t *) lv_event_get_data());
		if (key == LV_KEY_HOME && ctx_is_active())
		{
		    fm_record_cache_t * file = &local->filenames[index];

			strncpy(filemanager_active_fname,  file->name, sizeof(filemanager_active_fname) - 1);

			ctx_open(0);
		}
	}

	//Suppress default handler
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

static int fm_sort_name(const void * a, const void * b)
{
	//Take mode into account to place directories first
    fm_record_cache_t * file_a = (fm_record_cache_t *)a;
    fm_record_cache_t * file_b = (fm_record_cache_t *)b;

    if (RED_S_ISDIR(file_a->mode) == RED_S_ISDIR(file_b->mode))
        return strcmp(file_a->name, file_b->name);
    if (RED_S_ISDIR(file_a->mode) && !RED_S_ISDIR(file_b->mode))
        return +1;
    if (!RED_S_ISDIR(file_a->mode) && RED_S_ISDIR(file_b->mode))
        return -1;
    return 0;
}

static int fm_sort_date(const void * a, const void * b)
{
    fm_record_cache_t * file_a = (fm_record_cache_t *)a;
    fm_record_cache_t * file_b = (fm_record_cache_t *)b;

    if (file_a->date < file_b->date)
        return -1;
    if (file_a->date > file_b->date)
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
	local->filenames_count = 0;
	local->inode = 0;
	strncpy(local->path, path, PATH_LEN);

    local->cb(FM_CB_INIT, path);

	REDDIR * dir = red_opendir(path);
	if (dir != NULL)
	{

		uint16_t cnt = 0;
        while (cnt < FM_FILE_MAX_COUNT)
        {
            REDDIRENT * entry = red_readdir(dir);
            if (entry == NULL)
            	break;

            //hide system files
            if (entry->d_name[0] == '.')
                continue;

            if (local->flags & FM_FLAG_FILTER)
            {
                char new_path[PATH_LEN];
                snprintf(new_path, sizeof(new_path), "%s/%s", local->path, entry->d_name);

                if (!local->cb(FM_CB_FILTER, new_path))
                    continue;
            }

            if (RED_S_ISDIR(entry->d_stat.st_mode))
            {
                if (local->flags & FM_FLAG_HIDE_DIR)
                    continue;
            }
            else
            {
                if (local->flags & FM_FLAG_HIDE_FILE)
                    continue;
            }

            local->filenames[cnt].date = entry->d_stat.st_atime;
            local->filenames[cnt].mode = entry->d_stat.st_mode;
            strncpy(&local->filenames[cnt].name, entry->d_name, REDCONF_NAME_MAX);
            local->filenames_count++;

            cnt++;
        }
        red_closedir(dir);

        if (cnt == 0)
        {
            gui_list_note_add_entry(local->list, "Nothing to show", LIST_NOTE_COLOR);
            gui_set_dummy_event_cb(local->list, filemanager_dummy_cb);
        }
        else
        {
            if (local->flags & FM_FLAG_SORT_NAME)
            {
                qsort(local->filenames, cnt, sizeof(fm_record_cache_t), fm_sort_name);
            }
            if (local->flags & FM_FLAG_SORT_DATE)
            {
                qsort(local->filenames, cnt, sizeof(fm_record_cache_t), fm_sort_date);
            }
            for ( int i = 0; i < cnt; i++ )
            {
                char name[PATH_LEN];
                if (RED_S_ISDIR(local->filenames[i].mode))
                {
                    snprintf(name, sizeof(name), LV_SYMBOL_DIRECTORY " %s", local->filenames[i].name);
                }
                else
                {
                    if (local->flags & FM_FLAG_SHOW_EXT)
                    {
                        strcpy(name, local->filenames[i].name);
                    }
                    else
                    {
                        filemanager_get_filename_no_ext(name, local->filenames[i].name);
                    }
                }
                gui_list_text_add_entry(local->list, name);
            }
        }

	}
	else
	{
        gui_list_note_add_entry(local->list, "Directory not found", LIST_NOTE_COLOR);
        gui_set_dummy_event_cb(local->list, filemanager_dummy_cb);
	}

	local->cb(FM_CB_APPEND, path);

    //get dir inode
    int32_t f = red_open(path, RED_O_RDONLY);
    if (f > 0)
    {
        REDSTAT stat;
        red_fstat(f, &stat);
        red_close(f);

        local->inode = stat.st_ino;
    }

	if (local->inode != 0)
	{
        //focus previous position
        uint16_t last_pos = filemanager_retrive_inode_pos(local->inode);
        lv_obj_t * obj = gui_list_get_entry(last_pos);
        if (obj != NULL)
        {
            gui_focus_child(obj, NULL);
        }
        else if (last_pos > 0)
        {
            obj = gui_list_get_entry(gui_list_size() - 1);
            if (obj != NULL)
            {
                gui_focus_child(obj, NULL);
            }
        }
	}
}

//reopen filemanager in the same dir, useful when removing or adding files
void filemanager_refresh()
{
    gui_local_vars_t * old = gui_switch_task(&gui_filemanager, LV_SCR_LOAD_ANIM_NONE);
    filemanager_open(old->path, old->level, old->back, old->flags, old->cb);
}

