/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */

#include "blank.h"
#include "gui/statusbar.h"

REGISTER_TASK_IS(blank);

lv_obj_t * blank_init(lv_obj_t * par)
{
    lv_obj_t * obj = lv_obj_create(par, NULL);
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_size(obj, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(obj, 0, 0);

    statusbar_hide();

    return obj;
}

void blank_stop()
{
    statusbar_show();
}
