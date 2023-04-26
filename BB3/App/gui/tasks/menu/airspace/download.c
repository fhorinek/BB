#include <gui/tasks/menu/airspace/airspace.h>
#include <gui/tasks/menu/airspace/airspace.h>
#include "fc/airspaces/airspace.h"
#include "drivers/esp/download/slot.h"
#include "drivers/esp/protocol.h"
#include "gui/dialog.h"
#include "fc/airspaces/airspace.h"
#include "drivers/rtc.h"
#include "etc/epoch.h"

#include "gui/gui_list.h"

#include "etc/format.h"

#define MAX_RECORDS 64

REGISTER_TASK_IS(airspace_download,
    bool change;
    lv_obj_t * list;
    uint8_t ids[MAX_RECORDS];
    uint8_t index;
    uint8_t slot_id;
    char * as_name;
);

void airspace_download_progress_cb(uint8_t res, void *data)
{
    UNUSED(data);

    if (res == dialog_res_cancel)
    {
        esp_http_stop(local->slot_id);
    }
}



void airspace_apply_cb(uint8_t res, void * data)
{
    UNUSED(data);

    char * opt_data = dialog_get_opt_data();

    if (res == dialog_res_yes)
    {
        config_set_text(&profile.airspace.filename, opt_data);

        dialog_show(_("Loading airspace..."), opt_data, dialog_progress, NULL);
        dialog_progress_spin();

        gui_low_priority(true);

        xTaskCreate((TaskFunction_t)airspace_load_task, "as_load_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);
    }

    tfree(opt_data);
}

void airspace_download_get_file_cb(uint8_t res, download_slot_t *ds)
{
    INFO("airspace_download_get_file_cb cb %u", res);

    if (res == DOWNLOAD_SLOT_PROGRESS)
    {
        dialog_progress_set_progress((ds->pos * 100) / ds->size);
    }
    else if (res == DOWNLOAD_SLOT_COMPLETE)
    {
        dialog_close();
        download_slot_file_data_t *data = (download_slot_file_data_t*) ds->data;
        char path[64];
        char tmp_path[TEMP_NAME_LEN];

        get_tmp_path(tmp_path, data->tmp_id);

        snprintf(path, sizeof(path), "%s/%s.txt", PATH_AIRSPACE_DIR, local->as_name);

        red_unlink(path);
        red_rename(tmp_path, path);
        red_unlink(tmp_path);

        dialog_show(_("Load this airspace?"), "", dialog_yes_no, airspace_apply_cb);

        char *opt_data = tmalloc(strlen(local->as_name) + 5);
        sprintf(opt_data, "%s.txt", local->as_name);
        dialog_add_opt_data(opt_data);
    }
    else
    {
        dialog_downloads_error(res);
    }
}

void airspace_download(char *name, uint8_t id)
{

    char url[256];

    char date[32];

    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t wday;
    uint8_t month;
    uint16_t year;
    datetime_from_epoch(rtc_get_epoch(), &sec, &min, &hour, &day, &wday, &month, &year);
    snprintf(date, sizeof(date), "%u-%u-%u-SR", year, month, day);

    snprintf(url, sizeof(url), "https://airspace.xcontest.org/download/export?start=%s&end=SS&countryids=%u&exportType=openair", date, id);

    local->slot_id = esp_http_get(url, DOWNLOAD_SLOT_TYPE_FILE, airspace_download_get_file_cb);
    dialog_show(_("Downloading airspace"), "", dialog_progress, airspace_download_progress_cb);
    dialog_progress_spin();
    dialog_progress_set_subtitle(name);

    local->as_name = name;
}

bool airspace_download_cb(lv_obj_t *obj, lv_event_t event, uint16_t index)
{
    if (event == LV_EVENT_CLICKED)
    {
        airspace_download(gui_list_text_get_value(obj), local->ids[index]);
    }

    return true;
}

static void airspace_list_id_cb(char *key, char *value)
{
    unsigned int tmp = 0;

    if (local->index < MAX_RECORDS)
    {
        gui_list_text_add_entry(local->list, key, 0);
        sscanf(value, "%u", &tmp);

        local->ids[local->index] = tmp;
        local->index++;
    }
    else
    {
        ERR("Unexpected number of lines!");
    }
}

static lv_obj_t* airspace_download_init(lv_obj_t *par)
{
    local->change = false;
    local->index = 0;

    local->list = gui_list_create(par, _("Download airspace"), &gui_airspace, airspace_download_cb);

    db_iterate(PATH_AS_ID_FILE, airspace_list_id_cb);

    return local->list;
}

static void airspace_download_stop()
{
    if (local->change)
    {
        //reload
    }
}

