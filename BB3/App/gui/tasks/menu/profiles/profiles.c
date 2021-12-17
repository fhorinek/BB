#include "profiles.h"

#include "gui/tasks/menu/profiles/profiles.h"
#include "gui/tasks/menu/profiles/pilot.h"
#include "gui/tasks/menu/settings.h"

#include "gui/statusbar.h"
#include "gui/dialog.h"
#include "gui/ctx.h"
#include "gui/gui_list.h"
#include "gui/tasks/filemanager.h"

REGISTER_TASK_I(profiles);

void profile_pilot_open_fm(bool anim);

void profiles_pilot_fm_remove_cb(uint8_t res, void * opt_data)
{
    char * path = opt_data;
    if (res == dialog_res_yes)
    {
        f_unlink(path);

        char name[PATH_LEN];
        filemanager_get_filename_no_ext(name, path);

        char text[64];
        snprintf(text, sizeof(text), "Pilot profile '%s' deleted", name);
        statusbar_msg_add(STATUSBAR_MSG_INFO, text);

        //refresh
        profile_pilot_open_fm(false);
    }

    free(path);
}

void profile_pilot_fm_rename_cb(uint8_t res, void * opt_data)
{
    char * old_path = dialog_get_opt_data();
    char old_name[PATH_LEN] = {0};
    filemanager_get_filename_no_ext(old_name, old_path);

    if (res == dialog_res_yes)
    {
        char * new_name = opt_data;
        char new_path[PATH_LEN] = {0};
        str_join(new_path, 4, PATH_PILOT_DIR, "/", new_name, ".cfg");

        if (file_exists(new_path))
        {
            char text[64];
            snprintf(text, sizeof(text), "Pilot profile with name '%s' already exist", new_name);
            dialog_show("Error", text, dialog_confirm, NULL);
        }
        else
        {
            if (strcmp(old_name, config_get_text(&config.pilot_profile)) == 0)
            {
                config_set_text(&config.pilot_profile, new_name);
            }

            f_rename(old_path, new_path);

            //refresh
            profile_pilot_open_fm(false);
        }

    }

    free(old_path);
}

bool profiles_pilot_fm_cb(uint8_t event, char * path)
{
    char name[strlen(path) + 1];
    filemanager_get_filename_no_ext(name, path);

    switch (event)
    {
        case FM_CB_SELECT:
        {
        	if (strcmp(name, config_get_text(&config.pilot_profile)) == 0)
        		return true;

            config_change_pilot(name);
            char text[64];
            snprintf(text, sizeof(text), "Pilot profile changed to '%s'", name);
            statusbar_msg_add(STATUSBAR_MSG_INFO, text);
            return true;
        }

        case FM_CB_FOCUS_FILE:
        {
            ctx_clear();
            ctx_add_option(LV_SYMBOL_PLUS " Add new");
            ctx_add_option(LV_SYMBOL_EDIT " Rename");
            ctx_add_option(LV_SYMBOL_COPY " Duplicate");
            ctx_add_option(LV_SYMBOL_TRASH " Delete");
            ctx_show();
            break;
        }

        case (0): //add new
        {
            for (uint8_t i = 1; i < 100; i++)
            {
                char new_name[32];
                snprintf(new_name, sizeof(new_name), "new_%u", i);
                char new_path[PATH_LEN] = {0};
                str_join(new_path, 4, PATH_PILOT_DIR, "/", new_name, ".cfg");

                if (file_exists(new_path))
                    continue;

                char path[PATH_LEN] = {0};
                str_join(path, 3, PATH_ASSET_DIR, "/", "pilot.cfg");
                copy_file(path, new_path);

                config_change_pilot(new_name);
                gui_switch_task(&gui_pilot, LV_SCR_LOAD_ANIM_MOVE_LEFT);
                return false;
            }
            break;
        }

        case (1): //rename
        {
            dialog_add_opt_param(name);
            dialog_show("Rename", "Set new pilot profile name", dialog_textarea, profile_pilot_fm_rename_cb);
            char * opt_data = malloc(strlen(path) + 1);
            strcpy(opt_data, path);
            dialog_add_opt_data(opt_data);
            break;
        }
        case (2): //duplicate
        {
            for (uint8_t i = 1; i < 100; i++)
            {
                char new_name[32];
                snprintf(new_name, sizeof(new_name), "%s_%u.cfg", name, i);
                char new_path[PATH_LEN] = {0};
                str_join(new_path, 3, PATH_PILOT_DIR, "/", new_name);

                if (file_exists(new_path))
                    continue;

                copy_file(path, new_path);

                //refresh
                profile_pilot_open_fm(false);
                break;
            }
            break;
        }
        case (3): //delete
        {
            if (strcmp(name, config_get_text(&config.pilot_profile)) == 0)
            {
                dialog_show("Error", "Cannot delete active pilot profile", dialog_confirm, NULL);
            }
            else
            {
                char text[64];
                sniprintf(text, sizeof(text), "Do you want to remove pilot profile '%s'", name);
                dialog_show("Confirm", text, dialog_yes_no, profiles_pilot_fm_remove_cb);
                char * opt_data = malloc(strlen(path) + 1);
                strcpy(opt_data, path);
                dialog_add_opt_data((void *)opt_data);
            }
            break;
        }
    }
    return true;
}

void profile_pilot_open_fm(bool anim)
{
    gui_switch_task(&gui_filemanager, (anim) ? LV_SCR_LOAD_ANIM_MOVE_LEFT : LV_SCR_LOAD_ANIM_NONE);
    filemanager_open(PATH_PILOT_DIR, 0, &gui_profiles, FM_FLAG_HIDE_DIR | FM_FLAG_FOCUS, profiles_pilot_fm_cb);
}

static bool profiles_pilot_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        profile_pilot_open_fm(true);
        return false;
    }
    return true;
}

void profile_profile_open_fm(bool anim);

void profiles_profile_fm_remove_cb(uint8_t res, void * opt_data)
{
    char * path = opt_data;
    if (res == dialog_res_yes)
    {
        f_unlink(path);

        char name[PATH_LEN];
        filemanager_get_filename_no_ext(name, path);

        path[0] = 0;
        str_join(path, 3, PATH_PAGES_DIR, "/", name);
        remove_dir(path);

        char text[64];
        snprintf(text, sizeof(text), "Flight profile '%s' deleted", name);
        statusbar_msg_add(STATUSBAR_MSG_INFO, text);

        //refresh
        profile_profile_open_fm(false);
    }

    free(opt_data);
}

void profile_profile_fm_rename_cb(uint8_t res, void * opt_data)
{
    char * old_path = dialog_get_opt_data();
    char old_name[PATH_LEN] = {0};
    filemanager_get_filename_no_ext(old_name, old_path);


    if (res == dialog_res_yes)
    {
        char * new_name = opt_data;
        char new_path[PATH_LEN] = {0};
        str_join(new_path, 4, PATH_PROFILE_DIR, "/", new_name, ".cfg");

        if (file_exists(new_path))
        {
            char text[64];
            snprintf(text, sizeof(text), "Flight profile with name '%s' already exist", new_name);
            dialog_show("Error", text, dialog_confirm, NULL);
        }
        else
        {
            if (strcmp(old_name, config_get_text(&config.flight_profile)) == 0)
            {
                config_set_text(&config.flight_profile, new_name);
            }

            f_rename(old_path, new_path);

            char old_name[PATH_LEN];
            filemanager_get_filename_no_ext(old_name, old_path);
            old_path[0] = 0;
            str_join(old_path, 3, PATH_PROFILE_DIR, "/", old_name);
            new_path[0] = 0;
            str_join(new_path, 3, PATH_PROFILE_DIR, "/", new_name);
            f_rename(old_path, new_path);

            //refresh
            profile_profile_open_fm(false);
        }

    }

    free(old_path);
}

bool profiles_profile_fm_cb(uint8_t event, char * path)
{
    char name[strlen(path) + 1];
    filemanager_get_filename_no_ext(name, path);

    switch (event)
    {
        case FM_CB_SELECT:
        {
        	if (strcmp(name, config_get_text(&config.flight_profile)) == 0)
        		return true;

            config_change_profile(name);
            char text[64];
            snprintf(text, sizeof(text), "Flight profile changed to '%s'", name);
            statusbar_msg_add(STATUSBAR_MSG_INFO, text);
            return true;
        }

        case FM_CB_FOCUS_FILE:
        {
            ctx_clear();
            ctx_add_option(LV_SYMBOL_PLUS " Add new");
            ctx_add_option(LV_SYMBOL_EDIT " Rename");
            ctx_add_option(LV_SYMBOL_COPY " Duplicate");
            ctx_add_option(LV_SYMBOL_TRASH " Delete");
            ctx_show();
            break;
        }

        case (0): //add new
        {
            for (uint8_t i = 1; i < 100; i++)
            {
                char new_name[32];
                snprintf(new_name, sizeof(new_name), "new_%u", i);
                char new_path[PATH_LEN] = {0};
                str_join(new_path, 4, PATH_PROFILE_DIR, "/", new_name, ".cfg");

                if (file_exists(new_path))
                    continue;

                char path[PATH_LEN] = {0};
                str_join(path, 3, PATH_ASSET_DIR, "/", "profile.cfg");
                copy_file(path, new_path);

                new_path[0] = 0;
                str_join(new_path, 3, PATH_PAGES_DIR, "/", new_name);
                remove_dir(new_path);

                config_change_profile(new_name);
                config_store_all();

                gui_switch_task(&gui_profiles, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
                break;
            }
            break;
        }

        case (1): //rename
        {
            dialog_add_opt_param(name);
            dialog_show("Rename", "Set new flight profile name", dialog_textarea, profile_profile_fm_rename_cb);
            char * opt_data = malloc(strlen(path) + 1);
            strcpy(opt_data, path);
            dialog_add_opt_data(opt_data);
            break;
        }
        case (2): //duplicate
        {
            for (uint8_t i = 1; i < 100; i++)
            {
                char new_name[32];
                snprintf(new_name, sizeof(new_name), "%s_%u", name, i);
                char new_path[PATH_LEN] = {0};
                str_join(new_path, 4, PATH_PROFILE_DIR, "/", new_name, ".cfg");

                if (file_exists(new_path))
                    continue;

                copy_file(path, new_path);

                char old_path[PATH_LEN] = {0};
                str_join(old_path, 3, PATH_PAGES_DIR, "/", name);
                new_path[0] = 0;
                str_join(new_path, 3, PATH_PAGES_DIR, "/", new_name);

                copy_dir(old_path, new_path);

                //refresh
                profile_profile_open_fm(false);
                break;
            }
            break;
        }
        case (3): //delete
        {
            if (strcmp(name, config_get_text(&config.flight_profile)) == 0)
            {
                dialog_show("Error", "Cannot delete active flight profile", dialog_confirm, NULL);
            }
            else
            {
                char text[64];
                sniprintf(text, sizeof(text), "Do you want to remove flight profile '%s'", name);
                dialog_show("Confirm", text, dialog_yes_no, profiles_profile_fm_remove_cb);
                char * opt_data = malloc(strlen(path) + 1);
                strcpy(opt_data, path);
                dialog_add_opt_data((void *)opt_data);
            }
            break;
        }
    }
    return true;
}

void profile_profile_open_fm(bool anim)
{
    gui_switch_task(&gui_filemanager, (anim) ? LV_SCR_LOAD_ANIM_MOVE_LEFT : LV_SCR_LOAD_ANIM_NONE);
    filemanager_open(PATH_PROFILE_DIR, 0, &gui_profiles, FM_FLAG_HIDE_DIR | FM_FLAG_FOCUS, profiles_profile_fm_cb);
}

static bool profiles_flight_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        profile_profile_open_fm(true);
        return false;
    }
    return true;
}


static lv_obj_t * profiles_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Pilot & Flight profile", &gui_settings, NULL);

//	gui_list_auto_entry(list, "Ask on startup", &config.ask_on_start, NULL);

	char name[64];
	snprintf(name, sizeof(name), "%s (%s)", config_get_text(&pilot.name), config_get_text(&config.pilot_profile));
	lv_obj_t * pilot = gui_list_info_add_entry(list, "Pilot profile", name);
	gui_config_entry_add(pilot, CUSTOM_CB, profiles_pilot_cb);

    lv_obj_t * flight = gui_list_info_add_entry(list, "Flight profile", config_get_text(&config.flight_profile));
    gui_config_entry_add(flight, CUSTOM_CB, profiles_flight_cb);


	gui_list_auto_entry(list, "Edit pilot profile", NEXT_TASK, &gui_pilot);

    return list;
}



