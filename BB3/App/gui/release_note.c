/*
 * release_note.c
 *
 *  Created on: 4. 3. 2022
 *      Author: horinek
 */
#include "release_note.h"
#include "gui.h"
#include "dialog.h"
#include "gui_list.h"
#include "drivers/rev.h"

#include "gui/tasks/menu/system/firmware.h"

static void confirm_devel_ch(uint8_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        gui_list_set_pos(&gui_firmware, 3);
        gui_switch_task(&gui_firmware, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    }
}

static void confirm_channel(uint8_t res, void * data)
{
    char fw_str[20];
    rev_get_sw_string(fw_str);

    if (fw_str[0] == 'D')
    {
        if (!DEVEL_ACTIVE)
        {
            dialog_show(LV_SYMBOL_WARNING " Warning " LV_SYMBOL_WARNING,
                    _("Your device is currently running a development firmware.\n\nIt may be highly unstable and should be used only by developers or testers.\n\nWe highly encourage you to switch to an official released version."),
                    dialog_yes_no, confirm_devel_ch);
        }
    }

    if (fw_str[0] == 'T')
    {
        //TODO: switch to release
    }
}

void release_note_show()
{
    #define RELEASE_NOTE_BUFF_SIZE (1024 * 8)
    char * buff = ps_malloc(RELEASE_NOTE_BUFF_SIZE);

    int32_t f = red_open(PATH_RELEASE_NOTE, RED_O_RDONLY);
    if (f > 0)
    {
        red_read(f, buff, RELEASE_NOTE_BUFF_SIZE);
        red_close(f);

        char title[64];
        char fw_str[20];
        rev_get_sw_string(fw_str);
        snprintf(title, sizeof(title), _("Firmware updated\n to %s"), fw_str);
        dialog_show(title, buff, dialog_release_note, confirm_channel);
    }

    ps_free(buff);
}
