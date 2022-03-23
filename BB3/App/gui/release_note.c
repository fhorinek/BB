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
    if (config_get_select(&config.system.fw_channel) == FW_DEVEL)
    {
        if (!DEVEL_ACTIVE)
        {
            dialog_show(LV_SYMBOL_WARNING " Warning " LV_SYMBOL_WARNING,
                    "Your device is currently set to development firmware channel.\n\n"
                    "Firmware on this channel is highly unstable and should be used only by developers.\n\n"
                    "We highly encourage you to switch to testing branch.",
                    dialog_yes_no, confirm_devel_ch);
        }
    }

    if (config_get_select(&config.system.fw_channel) == FW_TESTING)
    {
        //TODO: switch to release
    }
}

void release_note_show()
{
    #define RELEASE_NOTE_BUFF_SIZE (1024 * 8)
    char * buff = ps_malloc(RELEASE_NOTE_BUFF_SIZE);

    FIL f;
    if (f_open(&f, PATH_RELEASE_NOTE, FA_READ) == FR_OK)
    {
        UINT br;
        f_read(&f, buff, RELEASE_NOTE_BUFF_SIZE, &br);
        f_close(&f);

        char title[64];
        char fw_str[20];
        rev_get_sw_string(fw_str);
        snprintf(title, sizeof(title), "Firmware updated\n to %s", fw_str);
        dialog_show(title, buff, dialog_release_note, confirm_channel);
    }

    ps_free(buff);
}
