/*
 * map.c
 *
 *  Created on: Mar 4, 2021
 *      Author: horinek
 */

//#define DEBUG_LEVEL DBG_DEBUG
#include "map_thread.h"
#include "tile.h"

#include "fc/fc.h"
#include "etc/geo_calc.h"

#define MAP_BUFFER_SIZE	(MAP_W * MAP_H * sizeof(lv_color_t))

// This is the center of the map to load with the given zoom factor
static int32_t map_static_latitude, map_static_longitude;
static uint8_t map_static_zoom;
static bool map_static;

/**
 * Tell map_thread what tiles to load and give the static position and zoom
 * factor. In "static" mode, this position will be used instead of GPS.
 *
 * @param latitude latitude of the center point of the map
 * @param longitude longitude of the center point of the map
 * @param zoom the zoom factor of the map
 */
void map_set_static_pos(int32_t latitude, int32_t longitude, uint8_t zoom)
{
	map_static_latitude = latitude;
	map_static_longitude = longitude;
	map_static_zoom = zoom;
	map_static = true;
}

/**
 * Tell map thread to take current GPS position and configured zoom mode
 * for loading tiles.
 */
void map_set_automatic_pos()
{
	map_static = false;
}

void map_init()
{
	map_set_automatic_pos();

	for (uint8_t i = 0; i < MAP_CHUNKS; i++)
    {
    	gui.map.chunks[i].buffer = ps_malloc(MAP_BUFFER_SIZE);
    	gui.map.chunks[i].ready = false;
    }

	gui_lock_acquire();
	gui.map.magic = 0xFF;
    gui.map.canvas = lv_canvas_create(lv_layer_sys(), NULL);
    gui.map.canvas_buffer = malloc(MAP_BUFFER_SIZE);
    lv_canvas_set_buffer(gui.map.canvas, gui.map.canvas_buffer, MAP_W, MAP_H, LV_IMG_CF_TRUE_COLOR);

    gui.map.poi_size = 0;
    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        gui.map.poi[i].chunk = 0xFF;
        gui.map.poi[i].magic = 0xFF;
    }

    lv_obj_set_hidden(gui.map.canvas, true);
    gui_lock_release();

    airspace_init_buffer();
}

void mk_gen_order(uint8_t * arr)
{
    typedef struct {
        uint8_t index;
        uint16_t val;
    } gen_order_helper_t;

    int cmpfunc (const void * a, const void * b)
    {
       return ((gen_order_helper_t *)a)->val - ((gen_order_helper_t *)b)->val;
    }

    gen_order_helper_t gen_order_helper[MAP_CHUNKS];

    for (uint8_t i = 0; i < MAP_CHUNKS; i++)
    {
        int16_t a = - (MAP_COLS / 2) + (i % MAP_COLS);
        int16_t b = - (MAP_ROWS / 2) + (i / MAP_COLS);

        gen_order_helper[i].index = i;
        gen_order_helper[i].val = sqrt(pow(a, 2) + pow(b, 2)) * 100 + i;
    }

    qsort(gen_order_helper, MAP_CHUNKS, sizeof(gen_order_helper_t), cmpfunc);

    for (uint8_t i = 0; i < MAP_CHUNKS; i++)
    {
        arr[i] = gen_order_helper[i].index;
    }
}

void thread_map_start(void *argument)
{
    UNUSED(argument);

    system_wait_for_handle(&thread_map);

    INFO("Started");

    map_init();

    osDelay(100);
    bool load_airspace = true;


    uint8_t gen_order[MAP_CHUNKS];
    mk_gen_order(gen_order);

    while(!system_power_off)
    {
    	uint8_t zoom;

    	if ( map_static )
    	{
    		gui.map.lat = map_static_latitude;
    		gui.map.lon = map_static_longitude;
    		zoom = map_static_zoom;
    	}
    	else
    	{
    		if (fc.gnss.fix == 0)
    		{
    		    gui.map.lat = config_get_big_int(&profile.ui.last_lat);
    		    gui.map.lon = config_get_big_int(&profile.ui.last_lon);
    		}
    		else
    		{
    		    gui.map.lat = fc.gnss.latitude;
    		    gui.map.lon = fc.gnss.longtitude;
    		}
            zoom = config_get_int(&profile.map.zoom_flight);
    	}

    	if (load_airspace)
    	{
    	    //need to know lat, lon
    	    airspace_load_parallel();
    	    load_airspace = false;
    	}

        airspace_step();

        uint8_t old_magic = gui.map.magic;

    	int32_t step_x;
    	int32_t step_y;
    	geo_get_steps(gui.map.lat, zoom, &step_x, &step_y);

    	//get vectors
    	int32_t step_lon = (gui.map.lon > 0 ? 1 : -1) * (MAP_W * step_x);
    	int32_t step_lat = (gui.map.lat > 0 ? 1 : -1) * (MAP_H * step_y);

    	int32_t c_lon;
    	int32_t c_lat;
    	align_to_cache_grid(gui.map.lon, gui.map.lat, zoom, &c_lon, &c_lat);

    	typedef struct
    	{
    		int32_t lon;
    		int32_t lat;

    		uint8_t chunk;
    		bool reload;
    		uint8_t _pad[2];
    	} tile_info_t;

    	tile_info_t tiles[MAP_CHUNKS];

    	//find usable chunks around us
    	for (uint8_t i = 0; i < MAP_CHUNKS; i++)
    	{
    		uint8_t j = gen_order[i];

    		tiles[i].lon = c_lon - (MAP_COLS / 2) * step_lon + step_lon * (j % MAP_COLS);
    		tiles[i].lat = c_lat + (MAP_ROWS / 2) * step_lat - step_lat * (j / MAP_COLS);

    		tiles[i].chunk = tile_find_inside(tiles[i].lon, tiles[i].lat, zoom);

    		DBG("L %u = %u (%ld %ld)", i, tiles[i].chunk, tiles[i].lon, tiles[i].lat);
    	}
    	//DBG("");

        bool change = false;

    	//assign buffers to tiles
    	for (uint8_t i = 0; i < MAP_CHUNKS; i++)
    	{
    		if (tiles[i].chunk != 0xFF)
    		{
    			tiles[i].reload = false;
    			continue;
    		}

    		change = true;
			tiles[i].reload = true;

			//assign new chunk
    		for (uint8_t chunk = 0; chunk < MAP_CHUNKS; chunk++)
    		{
    			bool used = false;

    			//is chunk used?
    			for (uint8_t k = 0; k < MAP_CHUNKS; k++)
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
    				gui.map.chunks[chunk].ready = false;
    				break;
    			}
    		}
    	}

        if (change)
        {
            gui.map.magic = (gui.map.magic + 1) % 0xFF;
            change = false;
        }

    	//only cache first
        uint8_t loaded_cnt = 0;
    	for (uint8_t i = 0; i < MAP_CHUNKS; i++)
    	{
    		if (tiles[i].reload)
    		{
                gui_low_priority(true);
    			if (tile_load_cache(tiles[i].chunk, tiles[i].lon, tiles[i].lat, zoom))
    			{
    				tiles[i].reload = false;
    				change = true;
    				loaded_cnt++;
    				if (loaded_cnt > 5)
    				{
    				    loaded_cnt = 0;
    				    gui.map.magic = (gui.map.magic + 1) % 0xFF;
    				}
    			}
                gui_low_priority(false);
    		}
    	}

    	//last resort, regenerate
    	bool regeneration = false;
    	for (uint8_t i = 0; i < MAP_CHUNKS; i++)
    	{
    		if (tiles[i].reload)
    		{
    		    gui_low_priority(true);
    			if (tile_generate(tiles[i].chunk, tiles[i].lon, tiles[i].lat, zoom))
    			{
    			    change = true;
    			    regeneration = true;
    			}
    			gui_low_priority(false);
    			break;
    		}
    	}

        //draw airspace
    	loaded_cnt = 0;
        for (uint8_t i = 0; i < MAP_CHUNKS; i++)
        {
            if (!tiles[i].reload && !gui.map.chunks[tiles[i].chunk].airspace && fc.airspaces.valid)
            {
                gui_low_priority(true);
                osMutexAcquire(fc.airspaces.lock, WAIT_INF);
                if (tile_airspace(tiles[i].chunk, tiles[i].lon, tiles[i].lat, zoom))
                {
                    change = true;
                }
                osMutexRelease(fc.airspaces.lock);
                gui_low_priority(false);

                if (regeneration)
                {
                    break;
                }

                loaded_cnt++;
                if (loaded_cnt > 5)
                {
                    loaded_cnt = 0;
                    gui.map.magic = (gui.map.magic + 1) % 0xFF;
                }
            }
        }

        if (change)
        {
            gui.map.magic = (gui.map.magic + 1) % 0xFF;
        }


		if (gui.map.magic == old_magic)
		{
			osDelay(200);
		}

    }

    INFO("Done");
    osThreadSuspend(thread_map);
}
