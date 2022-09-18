/*
 * flightbook.c
 *
 * Show all flight logs recorded so far and let user select a flight to show.
 *
 *  Created on: Feb 27, 2022
 *      Author: tilmann@bubecks.de
 */

#include "flightbook.h"
#include "flightbook_flight.h"

#include "gui/tasks/menu/settings.h"
#include "gui/tasks/filemanager.h"

#include "gui/statusbar.h"
#include "gui/gui_list.h"
#include "gui/ctx.h"
#include "gui/dialog.h"

#include "etc/format.h"

void flightbook_flights_open_fm(bool anim)
{
    gui_switch_task(&gui_filemanager, (anim) ? LV_SCR_LOAD_ANIM_MOVE_LEFT : LV_SCR_LOAD_ANIM_NONE);
    filemanager_open(PATH_LOGS_DIR, 0, &gui_settings, FM_FLAG_FOCUS | FM_FLAG_SORT_DATE | FM_FLAG_SHOW_EXT, flightbook_flights_fm_cb);
}

static void flightbook_fm_remove_cb(uint8_t res, void * opt_data)
{
    char * path = opt_data;

    if (res == dialog_res_yes)
    {                                                                                                                                       
        red_unlink(path);                                                                                                                   

        char name[PATH_LEN];
        filemanager_get_filename(name, path);

        char text[64];
        snprintf(text, sizeof(text), "Flight '%s' deleted", name);
        statusbar_msg_add(STATUSBAR_MSG_INFO, text);                                                                                        

        flightbook_flights_open_fm(false);   // refresh
    }
    free(opt_data);
}

bool flightbook_flights_fm_cb(uint8_t event, char * path)
{
    switch (event)
    {
        case FM_CB_SELECT:
        case (0):
        {
        	uint8_t fm_level = filemanager_get_current_level();
            gui_switch_task(&gui_flightbook_flight, LV_SCR_LOAD_ANIM_MOVE_LEFT);
            flightbook_flight_open(path, fm_level);
            return false;
        }

        case FM_CB_FOCUS_FILE:
        {
            ctx_clear();
            ctx_add_option(LV_SYMBOL_EYE_OPEN " View");
            ctx_add_option(LV_SYMBOL_TRASH " Delete");
            ctx_show();
            break;
        }

        case (1): //delete
        {
            char name[strlen(path) + 1];
            filemanager_get_filename(name, path);

            char text[64];
            sniprintf(text, sizeof(text), "Do you want to remove flight '%s'", name);
            dialog_show("Confirm", text, dialog_yes_no, flightbook_fm_remove_cb);
            char * opt_data = malloc(strlen(path) + 1);
            strcpy(opt_data, path);
            dialog_add_opt_data((void *)opt_data);
            break;
        }
    }
    return true;
}



void flightbook_open()
{
    flightbook_flights_open_fm(true);
}


