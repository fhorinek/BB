/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" \
                            issue*/
#include <SDL2/SDL.h>
#include "lvgl/lvgl.h"
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/keyboard.h"
#include "lv_examples/lv_examples.h"

#include "map/common.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void hal_init(void);
static int tick_thread(void *data);
static void memory_monitor(lv_task_t *param);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t *kb_indev;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

#include "tile.h"

int32_t map_lon = 172348385;
int32_t map_lat = 480288198;
int16_t map_zoom = 200;
lv_obj_t * screen;

static lv_group_t *g;

static void my_event_cb(lv_obj_t *obj, lv_event_t event) {
	uint32_t *key;

	switch (event) {
	case LV_EVENT_CLICKED:
		printf("Clicked\n");

		lv_indev_t *indev = lv_indev_get_act();
		lv_point_t pos;
		lv_indev_get_point(indev, &pos);

		printf(" %u x %u\n", pos.x, pos.y);

		int32_t lon;
		int32_t lat;

		pix_to_point(pos, map_lon, map_lat, map_zoom, &lon, &lat, screen);

		map_lon = lon;
		map_lat = lat;

		printf("%d %d\n", lon, lat);

		break;
	case (LV_EVENT_KEY):
		key = lv_event_get_data();
		if (*key == 19) //+
		{
			map_zoom -= 10;
			map_zoom = max(0, map_zoom);
		}
		if (*key == 20) //-
		{
			map_zoom += 10;
			map_zoom = min(300, map_zoom);
		}
		break;

	default:
		return;
	}

//    create_tile(map_lon, map_lat, map_zoom, canvas);

	/*Etc.*/
}

void widget_map_init(lv_obj_t * base);
void widget_map_update(lv_obj_t * base, int32_t disp_lat, int32_t disp_lon);

void tile_align_to_cache_grid(int32_t lon, int32_t lat, uint16_t zoom, int32_t * c_lon, int32_t * c_lat);

#include "map/etc/geo_calc.h"

int main(int argc, char **argv) {
	(void) argc; /*Unused*/
	(void) argv; /*Unused*/

	mkdir(PATH_SYSTEM_DIR, 0777);
	mkdir(PATH_CACHE_DIR, 0777);
	mkdir(PATH_MAP_CACHE_DIR, 0777);

	/*Initialize LVGL*/
	lv_init();

	/*Initialize the HAL (display, input devices, tick) for LVGL*/
	hal_init();

	screen = lv_scr_act();


	lv_group_add_obj(g, screen);
	lv_obj_set_event_cb(screen, my_event_cb);

	map_init();
	widget_map_init(screen);

//	int32_t lon = 0;
//	uint16_t a= 0;
//	for (int32_t lat = 0; lat < 60 * GNSS_MUL; lat += 1700000)
//	{
//		lon = lat;
//
//		int32_t c_lat, c_lon;
//		tile_align_to_cache_grid(lon, lat, map_zoom, &c_lon, &c_lat);
//
//		int32_t step_x;
//		int32_t step_y;
//		geo_get_steps(c_lat, map_zoom, &step_x, &step_y);
//
//		//get bbox
//		uint32_t map_w = MAP_W * step_x;
//		uint32_t map_h = (MAP_H * step_y);
//		int32_t lon1 = c_lon - map_w / 2;
//		int32_t lon2 = c_lon + map_w / 2;
//		int32_t lat1 = c_lat + map_h / 2;
//		int32_t lat2 = c_lat - map_h / 2;
//
//		INFO("rectangle name=p%u bbox=%f,%f,%f,%f", a, lon / (float)GNSS_MUL, lat / (float)GNSS_MUL, lon / (float)GNSS_MUL + 0.01, lat / (float)GNSS_MUL + 0.01);
//		INFO("rectangle name=t%u bbox=%f,%f,%f,%f", a++, lon1 / (float)GNSS_MUL, lat1 / (float)GNSS_MUL, lon2 / (float)GNSS_MUL, lat2 / (float)GNSS_MUL);
//	}
//
//	return 0 ;

	while (1) {
		/* Periodically call the lv_task handler.
		 * It could be done in a timer interrupt or an OS task too.*/
		widget_map_update(screen, map_lat, map_lon);
		lv_task_handler();
		usleep(500);
		map_step();
	}

	return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics
 * library
 */
static void hal_init(void) {
	/* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
	monitor_init();

	/*Create a display buffer*/
	static lv_disp_buf_t disp_buf1;
	static lv_color_t buf1_1[LV_HOR_RES_MAX * 120];
	lv_disp_buf_init(&disp_buf1, buf1_1, NULL, LV_HOR_RES_MAX * 120);

	/*Create a display*/
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv); /*Basic initialization*/
	disp_drv.buffer = &disp_buf1;
	disp_drv.flush_cb = monitor_flush;
	lv_disp_drv_register(&disp_drv);

	/* Add the mouse as input device
	 * Use the 'mouse' driver which reads the PC's mouse*/
	mouse_init();
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv); /*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER;

	/*This function will be called periodically (by the library) to get the mouse position and state*/
	indev_drv.read_cb = mouse_read;
	lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv);

	/*Set a cursor for the mouse*/
	LV_IMG_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
	lv_obj_t *cursor_obj = lv_img_create(lv_scr_act(), NULL); /*Create an image object for the cursor */
	lv_img_set_src(cursor_obj, &mouse_cursor_icon); /*Set the image source*/
	lv_indev_set_cursor(mouse_indev, cursor_obj); /*Connect the image  object to the driver*/

	lv_indev_drv_t indev_drv_key;
	lv_indev_drv_init(&indev_drv_key); /*Basic initialization*/
	indev_drv_key.type = LV_INDEV_TYPE_KEYPAD;

	g = lv_group_create();

	/*This function will be called periodically (by the library) to get the mouse position and state*/
	indev_drv_key.read_cb = keyboard_read;
	lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_key);
	lv_indev_set_group(kb_indev, g);

	/* Tick init.
	 * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
	 * how much time were elapsed Create an SDL thread to do this*/
	SDL_CreateThread(tick_thread, "tick", NULL);

	/* Optional:
	 * Create a memory monitor task which prints the memory usage in
	 * periodically.*/
//  lv_task_create(memory_monitor, 5000, LV_TASK_PRIO_MID, NULL);
}

/**
 * A task to measure the elapsed time for LVGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data) {
	(void) data;

	while (1) {
		SDL_Delay(5); /*Sleep for 5 millisecond*/
		lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
	}

	return 0;
}

/**
 * Print the memory usage periodically
 * @param param
 */
static void memory_monitor(lv_task_t *param) {
	(void) param; /*Unused*/

	lv_mem_monitor_t mon;
	lv_mem_monitor(&mon);
	printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n",
			(int) mon.total_size - mon.free_size, mon.used_pct, mon.frag_pct,
			(int) mon.free_biggest_size);
}
