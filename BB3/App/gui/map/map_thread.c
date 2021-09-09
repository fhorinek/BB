/*
 * map.c
 *
 *  Created on: Mar 4, 2021
 *      Author: horinek
 */

#define DEBUG_LEVEL DEBUG_DBG
#include "map_thread.h"
#include "tile.h"

#include "fc/fc.h"


#define BORDER_W (280 / 2)
#define BORDER_H (400 / 2)

void map_init()
{
    gui.map.active_buffer = 0;
    gui.map.canvas = lv_canvas_create(lv_layer_sys(), NULL);

//    LV_IMG_CF_TRUE_COLOR
    gui.map.buffer[0] = ps_malloc(MAP_W * MAP_H * sizeof(lv_color_t));
    gui.map.buffer[1] = ps_malloc(MAP_W * MAP_H * sizeof(lv_color_t));
    lv_obj_set_hidden(gui.map.canvas, true);

    lv_canvas_set_buffer(gui.map.canvas, gui.map.buffer[0], MAP_W, MAP_H, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_fill_bg(gui.map.canvas, LV_COLOR_YELLOW, LV_OPA_COVER);
}

void thread_map_start(void *argument)
{
    INFO("Started");
    map_init();
    bool skip_first = true;
    osThreadSuspend(thread_map);

    osDelay(1000);

    while(!system_power_off)
    {
    	if (fc.gnss.fix != 3)
    	{
            osDelay(1000);
    		continue;
    	}

        osDelay(10000);


    	gui.map.active_buffer = !gui.map.active_buffer;

        gui.map.center_lon[gui.map.active_buffer] = fc.gnss.longtitude;
        gui.map.center_lat[gui.map.active_buffer] = fc.gnss.latitude;
        gui.map.zoom[gui.map.active_buffer] = config_get_int(&profile.map.zoom_flight);

        uint32_t start = HAL_GetTick();
        DBG("tile start");
        lv_canvas_set_buffer(gui.map.canvas, gui.map.buffer[gui.map.active_buffer], MAP_W, MAP_H, LV_IMG_CF_TRUE_COLOR);

        create_tile(gui.map.center_lon[gui.map.active_buffer],
        			gui.map.center_lat[gui.map.active_buffer],
					gui.map.zoom[gui.map.active_buffer],
					gui.map.canvas, skip_first);

        skip_first = false;

        gui.map.active_buffer = !gui.map.active_buffer;
        gui.map.magic++;

        DBG("tile duration %0.2fs", (HAL_GetTick() - start) / 1000.0);
    }

    INFO("Done");
    osThreadSuspend(thread_map);
}
