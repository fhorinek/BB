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
#include "etc/geo_calc.h"

#define MAP_BUFFER_SIZE	(MAP_W * MAP_H * sizeof(lv_color_t))

void map_init()
{
    for (uint8_t i = 0; i < 9; i++)
    {
    	gui.map.chunks[i].buffer = ps_malloc(MAP_BUFFER_SIZE);
    	gui.map.chunks[i].ready = false;
    }

	gui_lock_acquire();
	gui.map.magic = 0xFF;
    gui.map.canvas = lv_canvas_create(lv_layer_sys(), NULL);

    gui.map.poi_size = 0;
    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        gui.map.poi[i].chunk = 0xFF;
        gui.map.poi[i].magic = 0xFF;
    }

    lv_obj_set_hidden(gui.map.canvas, true);
    gui_lock_release();
}
extern int32_t map_lon;
extern int32_t map_lat;
extern uint16_t map_zoom;

void map_step()
{
	int32_t disp_lat;
	int32_t disp_lon;

	disp_lat = map_lat;
	disp_lon = map_lon;

    uint8_t old_magic = gui.map.magic;

    uint16_t zoom = map_zoom;

	int32_t step_x;
	int32_t step_y;
	geo_get_steps(disp_lat, zoom, &step_x, &step_y);

	//get vectors
	uint32_t step_lon = MAP_W * step_x;
	uint32_t step_lat = MAP_H * step_y;

	int32_t c_lon;
	int32_t c_lat;
	tile_align_to_cache_grid(disp_lon, disp_lat, zoom, &c_lon, &c_lat);

	typedef struct
	{
		int32_t lon;
		int32_t lat;

		uint8_t chunk;
		bool reload;
		uint8_t _pad[2];
	} tile_info_t;

	tile_info_t tiles[9];

	static uint8_t gen_order[9] = {4, 3, 5, 1, 7, 0, 2, 6, 8};
	for (uint8_t i = 0; i < 9; i++)
	{
		uint8_t j = gen_order[i];

		tiles[i].lon = c_lon - step_lon + step_lon * (j % 3);
		tiles[i].lat = c_lat - step_lat + step_lat * (j / 3);

		tiles[i].chunk = tile_find_inside(tiles[i].lon, tiles[i].lat, zoom);

//    		DBG("L %u = %u (%ld %ld)", i, tiles[i].chunk, tiles[i].lon, tiles[i].lat);
	}
//    	DBG("");

	//assign buffers to tiles
	for (uint8_t i = 0; i < 9; i++)
	{
		if (tiles[i].chunk != 0xFF)
		{
			tiles[i].reload = false;
			continue;
		}

		tiles[i].reload = true;

		//assign new chunk
		for (uint8_t chunk = 0; chunk < 9; chunk++)
		{
			bool used = false;

			//is chunk used?
			for (uint8_t k = 0; k < 9; k++)
			{
				if (tiles[k].chunk == chunk)
				{
					used = true;
					break;
				}
			}

			if (!used)
			{
			    //unload old chunk
			    tile_unload_pois(chunk);

				tiles[i].chunk = chunk;
				break;
			}
		}
	}


	//only cache first
//	for (uint8_t i = 0; i < 9; i++)
//	{
//		if (tiles[i].reload)
//		{
//			if (tile_load_cache(tiles[i].chunk, tiles[i].lon, tiles[i].lat, zoom))
//				tiles[i].reload = false;
//		}
//	}


	//last resort, regenerate
	for (uint8_t i = 0; i < 1; i++)
	{
		if (tiles[i].reload)
		{
			tile_generate(tiles[i].chunk, tiles[i].lon, tiles[i].lat, zoom);
			break;
		}
	}

//	if (gui.map.magic == old_magic)
//	{
//		osDelay(1000);
//	}
}

//void thread_map_start(void *argument)
//{
//	osThreadSuspend(thread_map);
//    INFO("Started");
//    map_init();
//
////    osDelay(1000);
//
//    while(!system_power_off)
//    {
//    	map_step();
//    }
//
//    INFO("Done");
//    osThreadSuspend(thread_map);
//}
