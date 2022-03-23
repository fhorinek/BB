/*
 * fw_notify.c
 *
 *  Created on: 24. 2. 2022
 *      Author: horinek
 */
#include "fw_notify.h"

#include "gui.h"
#include "statusbar.h"
#include "drivers/rev.h"

#include "drivers/esp/download/slot.h"

void update_info_cb(uint8_t res, download_slot_t * ds)
{
    if (res == DOWNLOAD_SLOT_PROGRESS)
        return;

    if (res == DOWNLOAD_SLOT_COMPLETE)
    {
        char msg[64];
        char new_fw[32];

        if (read_value(ds->data, "firmware", new_fw, sizeof(new_fw)))
        {
            char rev_str[20];
            rev_get_sw_string(rev_str);
            if (!(strncmp(rev_str, new_fw, strlen(rev_str)) == 0 || strlen(new_fw) == 0))
            {
                snprintf(msg, sizeof(msg), "New firmware\n %s is avalible.", new_fw);
                statusbar_msg_add(STATUSBAR_MSG_INFO, msg);
                statusbar_set_icon(BAR_ICON_FW, I_GREEN);
            }

        }
    }
}

void check_for_update(void * arg)
{
    char url[128];

    snprintf(url, sizeof(url), "%s/%s/", config_get_text(&config.system.server_url), config_get_select_text(&config.system.fw_channel));
    esp_http_get(url, DOWNLOAD_SLOT_TYPE_PSRAM, update_info_cb);
}
