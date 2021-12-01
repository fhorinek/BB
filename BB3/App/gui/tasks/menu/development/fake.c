/*
 * fake.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include <gui/tasks/menu/development/development.h>
#include <gui/tasks/menu/development/fake.h>
#include "gui/statusbar.h"
#include "drivers/psram.h"
#include "drivers/power/led.h"

REGISTER_TASK_IS(fake,
    uint8_t index;
    lv_obj_t * img;
    lv_color_t * buff;
);


void fake_load()
{
    DIR dir;
    FILINFO fno;
    static char path[] = "FAKE";
    char file[32];

    FRESULT res = f_opendir(&dir, path);
    if (res == FR_OK)
    {
        uint8_t i = 0;
        while (i <= local->index)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
            {
                local->index = i - 1;
                break;
            }

            if (fno.fsize <= 1)
                continue;

            if (!(fno.fattrib & AM_DIR))
            {
                sprintf(file, "%s/%s\n", path, fno.fname);
                i++;
            }
        }
    }
    else
    {
        return;
    }
    f_closedir(&dir);


    INFO("loading file %u:%s to buffer", local->index, file);

    FIL f;
    f_open(&f, file, FA_READ);

    UINT br;
    f_read(&f, local->buff, LV_HOR_RES * LV_VER_RES * sizeof(lv_color_t), &br);
    f_close(&f);

    lv_obj_invalidate(local->img);
}


static void fake_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CANCEL)
    {
        gui_switch_task(&gui_development, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    }

    if (event == LV_EVENT_PRESSED)
    {
        led_set_torch(100);
    }

    if (event == LV_EVENT_KEY)
    {
        uint32_t key = *((uint32_t *) lv_event_get_data());
        switch (key)
        {
            case(LV_KEY_RIGHT):
                local->index++;
                fake_load();
            break;

            case(LV_KEY_LEFT):
                local->index--;
                fake_load();
            break;

            case(LV_KEY_HOME):
                led_set_torch(0);
            break;
        }

    }

}


static lv_obj_t * fake_init(lv_obj_t * par)
{
    statusbar_hide();
    gui_set_dummy_event_cb(par,fake_event_cb);

    local->index = 0;

	local->img = lv_canvas_create(lv_layer_sys(), NULL);
	local->buff = ps_malloc(LV_HOR_RES * LV_VER_RES * sizeof(lv_color_t));

	lv_obj_set_pos(local->img, 0, 0);
	lv_obj_set_size(local->img, LV_HOR_RES, LV_VER_RES);
	lv_canvas_set_buffer(local->img, local->buff, LV_HOR_RES, LV_VER_RES, LV_IMG_CF_TRUE_COLOR);

	fake_load();

	return par;
}

static void fake_stop()
{
    lv_obj_del_async(local->img);
    ps_free(local->buff);
    statusbar_show();
    led_set_torch(0);
}

