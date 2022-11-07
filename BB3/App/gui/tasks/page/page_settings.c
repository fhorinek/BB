/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "page_settings.h"
#include "pages.h"
#include "widget_list.h"
#include "page_edit.h"
#include "page_autoset.h"

#include "gui/gui_list.h"
#include "gui/statusbar.h"
#include "gui/widgets/pages.h"
#include "gui/keyboard.h"
#include "gui/dialog.h"
#include "gui/tasks/filemanager.h"
#include "gui/ctx.h"
#include "gui/anim.h"

static lv_obj_t * static_prev_par = NULL;
static bool static_prev_mode = false;


REGISTER_TASK_I(page_settings,
		char page_name[PAGE_NAME_LEN + 1];

		lv_obj_t * name_entry;

		uint8_t page_index;
);

bool page_settings_load_page_fm_cb(uint8_t event, char * path);
void page_settings_close_preview();

void page_settings_open_fm()
{
    page_settings_close_preview();

    gui_switch_task(&gui_filemanager, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    char path[PATH_LEN] = {0};
    str_join(path, 3, PATH_PAGES_DIR, "/", config_get_text(&config.flight_profile));
    filemanager_open(path, 0, &gui_pages, FM_FLAG_FILTER | FM_FLAG_HIDE_DIR | FM_FLAG_SORT_NAME | FM_FLAG_FOCUS, page_settings_load_page_fm_cb);
}


void page_settings_update_pos(int8_t dir)
{
    uint8_t cnt = pages_get_count();

    if (dir != 0)
    {
        uint8_t old_index = local->page_index;
        uint8_t new_index = (local->page_index + dir + cnt) % cnt;

        config_set_text(&profile.ui.page[old_index], config_get_text(&profile.ui.page[new_index]));
        config_set_text(&profile.ui.page[new_index], local->page_name);

        local->page_index = new_index;
        config_set_int(&profile.ui.page_last, new_index);
    }

    char value[30];
    snprintf(value, sizeof(value), "Page name (%u / %u)", local->page_index + 1, cnt);
    gui_list_textbox_set_name(local->name_entry, value);
}

void page_settings_set_page_name(char * name, uint8_t index)
{
	strncpy(local->page_name, name, sizeof(local->page_name));
    local->page_index = index;

    gui_list_textbox_set_value(local->name_entry, name);

    page_settings_update_pos(0);
}

void page_settings_delete_cb(uint8_t res, void * data)
{
	if (res == dialog_res_yes)
	{
		page_delete(local->page_name);
		config_set_text(&profile.ui.page[local->page_index], "");
		pages_defragment();

		gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        char text[64];
        snprintf(text, sizeof(text), "Page '%s' removed", local->page_name);
        statusbar_msg_add(STATUSBAR_MSG_INFO, text);
	}
}

static bool page_setting_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
    UNUSED(index);

	if (event == LV_EVENT_CANCEL)
	{
		gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_LEFT);
	}

    if (obj == local->name_entry)
	{
        if (event == LV_EVENT_APPLY)
		{
			keyboard_hide();

			char * text = (char *)gui_list_textbox_get_value(local->name_entry);

			if (strcmp(text, local->page_name) == 0)
				return false;

			if (strlen(text) == 0)
			{
				gui_list_textbox_set_value(local->name_entry, local->page_name);
				return false;
			}

			if (!page_rename(local->page_name, text))
			{
				gui_list_textbox_set_value(local->name_entry, local->page_name);
				statusbar_msg_add(STATUSBAR_MSG_ERROR, "Already exists!");
			}
			else
			{
				config_set_text(&profile.ui.page[local->page_index], text);
				strcpy(local->page_name, text);
			}
		}

        if (event == LV_EVENT_LEAVE)
        {
            keyboard_hide();
            gui_list_textbox_set_value(local->name_entry, local->page_name);
            return false;
        }
	}

	return true;
}

static bool page_settings_edit_layout_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        gui_switch_task(&gui_page_edit, LV_SCR_LOAD_ANIM_MOVE_LEFT);
        page_edit_set_page_name(local->page_name, local->page_index);
    }
    return true;
}


static bool page_settings_autoset_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        gui_switch_task(&gui_page_autoset, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
        page_autoset_set_page_name(local->page_name, local->page_index);
    }
    return true;
}
static bool page_settings_add_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        //create new page
        for (uint8_t i = 1; i < 100; i++)
        {
            char new_name[16];
            snprintf(new_name, sizeof(new_name), "new_%u", i);
            if (page_create(new_name))
            {
                uint8_t index = pages_get_count();
                config_set_text(&profile.ui.page[index], new_name);
                config_set_int(&profile.ui.page_last, index);

                gui_switch_task(&gui_page_edit, LV_SCR_LOAD_ANIM_MOVE_LEFT);
                page_edit_set_page_name(new_name, index);

                char text[64];
                snprintf(text, sizeof(text), "Page '%s' created at position %u", new_name, index + 1);
                statusbar_msg_add(STATUSBAR_MSG_INFO, text);
                break;
            }
        }

    }
    return true;
}

static bool page_settings_remove_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        dialog_show("Remove page", "Do you want to remove this page", dialog_yes_no, page_settings_delete_cb);
    }
    return true;
}

static bool page_settings_move_left_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        page_settings_update_pos(-1);
    }
    return true;
}

static bool page_settings_move_right_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        page_settings_update_pos(+1);
    }
    return true;
}

static bool page_settings_unload_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        config_set_text(&profile.ui.page[local->page_index], "");
        pages_defragment();

        gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    }
    return true;
}

void page_settings_fm_rename_cb(uint8_t res, void * opt_data)
{
    char * old_path = dialog_get_opt_data();

    if (res == dialog_res_yes)
    {
        char * new_name = opt_data;
        char new_path[PATH_LEN];
        filemanager_get_path(new_path, old_path);
        str_join(new_path, 3, "/", new_name, ".pag");

        if (file_exists(new_path))
        {
            char text[64];
            snprintf(text, sizeof(text), "Page with name '%s' already exist", new_name);
            dialog_show("Error", text, dialog_confirm, NULL);
        }
        else
        {
            red_rename(old_path, new_path);

            //refresh
            page_settings_close_preview();
            filemanager_refresh();
        }

    }

    tfree(old_path);
}


void page_settings_fm_remove_cb(uint8_t res, void * opt_data)
{
    char * path = opt_data;

    if (res == dialog_res_yes)
    {
        red_unlink(path);

        char name[PATH_LEN];
        filemanager_get_filename_no_ext(name, path);

        char text[64];
        snprintf(text, sizeof(text), "Page '%s' deleted", name);
        statusbar_msg_add(STATUSBAR_MSG_INFO, text);

        //refresh
        page_settings_close_preview();
        filemanager_refresh();
    }

    tfree(path);
}

void page_settings_close_preview()
{
    if (static_prev_par != NULL)
    {
        lv_obj_del(static_prev_par);
        static_prev_par = NULL;
    }
}

void page_settings_open_preview(char * path)
{
    page_settings_close_preview();

    static_prev_par = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_pos(static_prev_par, 0, GUI_STATUSBAR_HEIGHT);
    lv_obj_set_size(static_prev_par, LV_HOR_RES, LV_VER_RES - GUI_STATUSBAR_HEIGHT);
    lv_obj_move_foreground(static_prev_par);

    page_layout_t page;
    widgets_load_from_file_abs(&page, path);
    widgets_init_page(&page, static_prev_par);
    widgets_deinit_page(&page);

    char page_name[PATH_LEN];
    filemanager_get_filename_no_ext(page_name, path);

    lv_obj_t * label = lv_label_create(static_prev_par, NULL);
    lv_label_set_text_fmt(label, LV_SYMBOL_LEFT " %s " LV_SYMBOL_RIGHT, page_name);
    lv_obj_set_style_local_bg_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_opa(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
    lv_obj_set_style_local_radius(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_pad_all(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_align(label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -50);
    lv_obj_move_foreground(label);
}

bool page_settings_load_page_fm_cb(uint8_t event, char * path)
{
    char name[strlen(path) + 1];
    filemanager_get_filename_no_ext(name, path);

    switch (event)
    {
        case FM_CB_BACK:
        {
            static_prev_mode = false;
            page_settings_close_preview();

            gui_switch_task(&gui_page_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
            uint8_t page_index = config_get_int(&profile.ui.page_last);
            char * page_name = config_get_text(&profile.ui.page[page_index]);
            page_settings_set_page_name(page_name, page_index);

            return false;
        }

        case FM_CB_FILTER:
        {
            for (uint8_t i = 0; i < PAGE_MAX_COUNT; i++)
            {
                if (strcmp(config_get_text(&profile.ui.page[i]), name) == 0)
                {
                    return false;
                }
            }
            return true;
        }

        case FM_CB_SELECT:
        {
            uint8_t page_cnt = pages_get_count();
            config_set_text(&profile.ui.page[page_cnt], name);
            config_set_int(&profile.ui.page_last, page_cnt);

            gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_LEFT);

            char text[64];
            snprintf(text, sizeof(text), "Page '%s' loaded to position %u", name, page_cnt + 1);
            statusbar_msg_add(STATUSBAR_MSG_INFO, text);

            page_settings_close_preview();

            return false;
        }

        case FM_CB_FOCUS_FILE:
        {
            if (static_prev_mode)
            {
                page_settings_open_preview(path);
            }

            ctx_clear();
            if (static_prev_mode)
                ctx_add_option(LV_SYMBOL_LIST " List");
            else
                ctx_add_option(LV_SYMBOL_EYE_OPEN " Preview");

            ctx_add_option(LV_SYMBOL_TRASH " Delete");
            ctx_add_option(LV_SYMBOL_COPY " Duplicate");
            ctx_add_option(LV_SYMBOL_EDIT " Rename");
            ctx_show();
            break;
        }

        case 0: //Preview
        {
            if (static_prev_mode)
            {
                page_settings_close_preview();
                static_prev_mode = false;
            }
            else
            {
                static_prev_mode = true;
            }
            break;
        }

        case 1: //delete
        {
            char text[64];
            sniprintf(text, sizeof(text), "Do you want to remove page '%s'", name);
            dialog_show("Confirm", text, dialog_yes_no, page_settings_fm_remove_cb);
            char * opt_data = tmalloc(strlen(path) + 1);
            strcpy(opt_data, path);
            dialog_add_opt_data((void *)opt_data);
            break;
        }

        case 2: //duplicate
        {
            for (uint8_t i = 1; i < 100; i++)
            {
                char new_name[32];
                snprintf(new_name, sizeof(new_name), "%s_%u.pag", name, i);
                char new_path[PATH_LEN];
                filemanager_get_path(new_path, path);
                str_join(new_path, 2, "/", new_name);

                if (file_exists(new_path))
                    continue;

                copy_file(path, new_path);

                //refresh
                page_settings_close_preview();
                filemanager_refresh();
                break;
            }
            break;
        }

        case 3: //rename
        {
            dialog_add_opt_param(name);
            dialog_show("Rename", "Set new page name", dialog_textarea, page_settings_fm_rename_cb);
            char * opt_data = tmalloc(strlen(path) + 1);
            strcpy(opt_data, path);
            dialog_add_opt_data(opt_data);
            break;
        }
    }


    return true;
}

static bool page_settings_load_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        page_settings_open_fm();

        return false;
    }
    return true;
}

void page_settings_open_copy_fm(bool anim);

bool page_settings_load_page_copy_fm_cb(uint8_t event, char * path)
{
    char name[strlen(path) + 1];
    filemanager_get_filename_no_ext(name, path);

    switch (event)
    {
		case FM_CB_CANCEL:
		{
			if (static_prev_mode)
			{
				static_prev_mode = false;
				page_settings_close_preview();

	            ctx_clear();
				ctx_add_option(LV_SYMBOL_EYE_OPEN " Preview");

				return false;
			}
			else
			{
				return true;
			}
		}

        case FM_CB_BACK:
        {
            static_prev_mode = false;
            page_settings_close_preview();

            gui_switch_task(&gui_page_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
            uint8_t page_index = config_get_int(&profile.ui.page_last);
            char * page_name = config_get_text(&profile.ui.page[page_index]);
            page_settings_set_page_name(page_name, page_index);

            return false;
        }

        case FM_CB_SELECT:
        {
            uint8_t page_cnt = pages_get_count();

            char dst[PATH_LEN] = {0};
            char new_name[PAGE_NAME_LEN];
            strcpy(new_name, name);
            str_join(dst, 6, PATH_PAGES_DIR, "/", config_get_text(&config.flight_profile), "/", new_name, ".pag");
            uint8_t i = 0;
            while (file_exists(dst) && i < 100)
            {
            	i++;
            	snprintf(new_name, sizeof(new_name), "%s_%u", name, i);
            	dst[0] = 0;
            	str_join(dst, 6, PATH_PAGES_DIR, "/", config_get_text(&config.flight_profile), "/", new_name, ".pag");
            }
            copy_file(path, dst);

            config_set_text(&profile.ui.page[page_cnt], new_name);
            config_set_int(&profile.ui.page_last, page_cnt);

            gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_LEFT);

            char text[64];
            snprintf(text, sizeof(text), "Page '%s' copied to position %u", new_name, page_cnt + 1);
            statusbar_msg_add(STATUSBAR_MSG_INFO, text);

            page_settings_close_preview();

            return false;
        }

        case FM_CB_FOCUS_FILE:
        {
            if (static_prev_mode)
            {
                page_settings_open_preview(path);
            }

            ctx_clear();
            if (static_prev_mode)
                ctx_add_option(LV_SYMBOL_LIST " List");
            else
                ctx_add_option(LV_SYMBOL_EYE_OPEN " Preview");

            ctx_show();
            break;
        }

        case 0: //Preview
        {
            if (static_prev_mode)
            {
                page_settings_close_preview();
                static_prev_mode = false;
            }
            else
            {
                static_prev_mode = true;
            }
            break;
        }
    }

    return true;
}

void page_settings_open_copy_fm(bool anim)
{
    page_settings_close_preview();

    gui_switch_task(&gui_filemanager, (anim) ? LV_SCR_LOAD_ANIM_MOVE_LEFT : LV_SCR_LOAD_ANIM_NONE);
    char path[PATH_LEN] = {0};
    strcpy(path, PATH_PAGES_DIR);
    filemanager_open(path, 0, &gui_pages, FM_FLAG_FOCUS | FM_FLAG_SORT_NAME, page_settings_load_page_copy_fm_cb);
}

static bool page_settings_copy_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        page_settings_open_copy_fm(true);

        return false;
    }
    return true;
}


static uint16_t hidden_pages_cnt()
{
    char path[PATH_LEN] = {0};
    str_join(path, 3, PATH_PAGES_DIR, "/", config_get_text(&config.flight_profile));

    REDDIR * dir = red_opendir(path);
    if (dir == NULL)
        return 0;

    uint16_t cnt = 0;
    while (true)
    {
        REDDIRENT * entry = red_readdir(dir);
        if (entry == NULL)
            break;

        char name[64];
        filemanager_get_filename_no_ext(name, entry->d_name);

        bool page_used = false;
        for (uint8_t i = 0; i < PAGE_MAX_COUNT; i++)
        {
            if (strcmp(config_get_text(&profile.ui.page[i]), name) == 0)
            {
                page_used = true;
                break;
            }
        }

        if (RED_S_ISDIR(entry->d_stat.st_mode) || page_used)
            continue;

        cnt++;
    }

    red_closedir(dir);
    return cnt;
}


static lv_obj_t * page_settings_init(lv_obj_t * par)
{
    help_set_base("Page/Settings");

    static_prev_par = NULL;
    static_prev_mode = false;
    uint8_t page_cnt = pages_get_count();

	lv_obj_t * list = gui_list_create(par, "Page settings", NULL, page_setting_cb);

    local->name_entry = gui_list_textbox_add_entry(list, "", "", PAGE_NAME_LEN);

	gui_list_auto_entry(list, LV_SYMBOL_EDIT " Edit page", CUSTOM_CB, page_settings_edit_layout_cb);
	gui_list_auto_entry(list, LV_SYMBOL_SETTINGS " Autoset", CUSTOM_CB, page_settings_autoset_cb);

	if (page_cnt > 1)
	{
		lv_obj_t * obj = gui_list_auto_entry(list, LV_SYMBOL_LEFT " Move left", CUSTOM_CB, page_settings_move_left_cb);
		lv_cont_set_fit2(obj, LV_FIT_NONE, LV_FIT_TIGHT);
		lv_obj_set_width(obj, lv_obj_get_width(obj) / 2);
        obj = gui_list_auto_entry(list, "Move right " LV_SYMBOL_RIGHT, CUSTOM_CB, page_settings_move_right_cb);
		lv_cont_set_fit2(obj, LV_FIT_NONE, LV_FIT_TIGHT);
		lv_obj_set_width(obj, lv_obj_get_width(obj) / 2);
	}

    if (page_cnt > 1)
    {
        gui_list_auto_entry(list, LV_SYMBOL_EYE_CLOSE " Hide page", CUSTOM_CB, page_settings_unload_cb);
        gui_list_auto_entry(list, LV_SYMBOL_TRASH " Remove page", CUSTOM_CB, page_settings_remove_cb);
    }

    if (page_cnt < PAGE_MAX_COUNT)
    {
        gui_list_spacer_add_entry(list, 14);

        if (hidden_pages_cnt() > 0)
        {
            gui_list_auto_entry(list, LV_SYMBOL_PLUS " Show hidden page", CUSTOM_CB, page_settings_load_cb);
        }

        gui_list_auto_entry(list, LV_SYMBOL_PLUS " Add empty page", CUSTOM_CB, page_settings_add_cb);
        gui_list_auto_entry(list, LV_SYMBOL_PLUS " Duplicate existing", CUSTOM_CB, page_settings_copy_cb);
    }

	return list;
}

